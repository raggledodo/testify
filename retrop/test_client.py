''' Test retrop client '''

from concurrent import futures
import unittest

import grpc

import proto.testdata_pb2 as testdata
import proto.record_pb2 as record
import proto.record_pb2_grpc as record_grpc

from retrop.generate import GenIO
import retrop.client as client

SERVER_PORT = 50055
_SERVER = None

latest_name = ""
latest_output = None

class MockService(record_grpc.DoraServicer):
    def AddTestcase(self, request, context):
        global latest_name
        global latest_output
        latest_name = request.name
        latest_output = request.results
        return record.Nothing()

    def RemoveTestcase(self, request, context):
        pass

    def ListTestcases(self, request, context):
        pass

    def GetTestcase(self, request, context):
        pass

def serve():
    global _SERVER
    _SERVER = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    record_grpc.add_DoraServicer_to_server(MockService(), _SERVER)
    _SERVER.add_insecure_port('[::]:' + str(SERVER_PORT))
    _SERVER.start()

class ClientTest(unittest.TestCase):
    def tearDown(self):
        latest_name = ""
        latest_output = None

    @classmethod
    def tearDownClass(cls):
        global _SERVER
        client.stop()
        try:
            th = _SERVER.stop(5)
            if th is not None:
                th.wait()
            _SERVER = None
        except:
            pass

    def assertArrEqual(self, arr1, arr2):
        self.assertEqual(len(arr1), len(arr2))
        for expect, got in zip(arr1, arr2):
            self.assertEqual(expect, got)

    def test_arr(self):
        expect_label = "arr"
        io = GenIO(expect_label)
        iarr = io.get_arr("int", int, 23, (-122, 192))
        darr = io.get_arr("double", float, 13, (-102, 92.2))

        out = [4, 12, 2, 19]
        io.set_arr(out, int)

        self.assertEqual(expect_label, latest_name)
        self.assertIsNotNone(latest_output)
        inputs = latest_output.inputs
        self.assertEqual(2, len(inputs))

        self.assertEqual("int", inputs[0].usage)
        self.assertEqual(testdata.INT64S, inputs[0].dtype)
        i64s = testdata.Int64s()
        inputs[0].data.Unpack(i64s)
        self.assertArrEqual(iarr, i64s.data)

        self.assertEqual("double", inputs[1].usage)
        self.assertEqual(testdata.DOUBLES, inputs[1].dtype)
        ds = testdata.Doubles()
        inputs[1].data.Unpack(ds)
        self.assertArrEqual(darr, ds.data)

        self.assertEqual(testdata.INT64S, latest_output.dtype)
        outr = testdata.Int64s()
        latest_output.output.Unpack(outr)
        self.assertArrEqual(out, outr.data)

    def test_str(self):
        expect_label = "str"
        io = GenIO(expect_label)
        s = io.get_str("strusage", 19)

        out = [5, 13, 3, 20]
        io.set_arr(out, int)

        self.assertEqual(expect_label, latest_name)
        self.assertIsNotNone(latest_output)
        inputs = latest_output.inputs
        self.assertEqual(1, len(inputs))

        self.assertEqual("strusage", inputs[0].usage)
        self.assertEqual(testdata.BYTES, inputs[0].dtype)
        bs = testdata.Bytes()
        inputs[0].data.Unpack(bs)
        self.assertEqual(s, bs.data.decode())

        self.assertEqual(testdata.INT64S, latest_output.dtype)
        outr = testdata.Int64s()
        latest_output.output.Unpack(outr)
        self.assertArrEqual(out, outr.data)

    def test_tree(self):
        expect_label = "tree"
        io = GenIO(expect_label)
        root, tr = io.get_tree("treeusage", 41)

        io.set_tree(root, tr)

        self.assertEqual(expect_label, latest_name)
        self.assertIsNotNone(latest_output)
        inputs = latest_output.inputs
        self.assertEqual(1, len(inputs))

    def test_graph(self):
        expect_label = "graph"
        io = GenIO(expect_label)
        gr = io.get_graph("graphusage", 43)

        io.set_graph(gr)

        self.assertEqual(expect_label, latest_name)
        self.assertIsNotNone(latest_output)
        inputs = latest_output.inputs
        self.assertEqual(1, len(inputs))

    def test_cgraph(self):
        expect_label = "cgraph"
        io = GenIO(expect_label)
        cg = io.get_cgraph("cgraphusage", 47)

        io.set_graph(cg)

        self.assertEqual(expect_label, latest_name)
        self.assertIsNotNone(latest_output)
        inputs = latest_output.inputs
        self.assertEqual(1, len(inputs))

if __name__ == "__main__":
    serve()
    assert(_SERVER is not None)
    client.init("0.0.0.0:" + str(SERVER_PORT))
    unittest.main()
