''' Test retrop client '''

from concurrent import futures
import unittest
import math
import time

import numpy as np

import grpc
from google.protobuf.empty_pb2 import Empty

import proto.testify_pb2 as testify
import proto.testify_pb2_grpc as testify_grpc

from retrop.generate import GenIO
import retrop.client as client

SERVER_PORT = 50055
_SERVER = None

latest_name = ""
latest_case = None

class MockService(testify_grpc.DoraServicer):
    def AddTestcase(self, request, context):
        global latest_name
        global latest_case
        latest_name = request.name
        latest_case = request.results
        return Empty()

    def RemoveTestcase(self, request, context):
        pass

    def ListTestcases(self, request, context):
        pass

    def GetTestcase(self, request, context):
        pass

def serve():
    global _SERVER
    _SERVER = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    testify_grpc.add_DoraServicer_to_server(MockService(), _SERVER)
    _SERVER.add_insecure_port('[::]:' + str(SERVER_PORT))
    _SERVER.start()

class ClientTest(unittest.TestCase):
    def tearDown(self):
        latest_name = ""
        latest_case = None

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

    def assertGraphEqual(self, gr, pbgr):
        self.assertEqual(gr.nverts, pbgr.nverts)
        encoding = pbgr.matrix
        nbytes = int(math.ceil(gr.nverts**2 / 8.0))
        self.assertEqual(nbytes, len(encoding))
        if isinstance(encoding[0], int):
            enc = lambda c : 1 & c >> (i % 8)
        else:
            enc = lambda c : 1 & ord(c) >> (i % 8)
        for y in range(gr.nverts):
            for x in range(gr.nverts):
                i = x + y * gr.nverts
                s = enc(encoding[int(i / 8)])
                self.assertEqual(gr.get(y, x), s)

    def test_arr(self):
        expect_label = "arr"
        io = GenIO(expect_label)
        iarr = io.get_arr("int", int, 23, (-122, 192))
        darr = io.get_arr("double", float, 13, (-102, 92.2))

        out = [4, 12, 2, 19]
        io.set_arr("stdout", out, int)
        io.send()

        self.assertEqual(expect_label, latest_name)
        self.assertIsNotNone(latest_case)
        inputs = latest_case.inputs
        self.assertEqual(2, len(inputs))
        outputs = latest_case.outputs
        self.assertEqual(1, len(outputs))

        self.assertTrue("int" in inputs)
        self.assertEqual(testify.INT64S, inputs["int"].dtype)
        i64s = testify.Int64s()
        inputs["int"].data.Unpack(i64s)
        self.assertArrEqual(iarr, i64s.data)

        self.assertTrue("double" in inputs)
        self.assertEqual(testify.DOUBLES, inputs["double"].dtype)
        ds = testify.Doubles()
        inputs["double"].data.Unpack(ds)
        self.assertArrEqual(darr, ds.data)

        self.assertTrue("stdout" in outputs)
        self.assertEqual(testify.INT64S, outputs["stdout"].dtype)
        outr = testify.Int64s()
        outputs["stdout"].data.Unpack(outr)
        self.assertArrEqual(out, outr.data)

    def test_str(self):
        expect_label = "str"
        io = GenIO(expect_label)
        s = io.get_str("strusage", 19)

        out = [5, 13, 3, 20]
        io.set_arr("stdout", out, int)
        io.send()

        self.assertEqual(expect_label, latest_name)
        self.assertIsNotNone(latest_case)
        inputs = latest_case.inputs
        self.assertEqual(1, len(inputs))
        outputs = latest_case.outputs
        self.assertEqual(1, len(outputs))

        self.assertTrue("strusage" in inputs)
        self.assertEqual(testify.BYTES, inputs["strusage"].dtype)
        bs = testify.Bytes()
        inputs["strusage"].data.Unpack(bs)
        self.assertEqual(s, bs.data.decode())

        self.assertTrue("stdout" in outputs)
        self.assertEqual(testify.INT64S, outputs["stdout"].dtype)
        outr = testify.Int64s()
        outputs["stdout"].data.Unpack(outr)
        self.assertArrEqual(out, outr.data)

    def test_tree(self):
        expect_label = "tree"
        io = GenIO(expect_label)
        root, tr = io.get_tree("treeusage", 41)

        io.set_tree("stdout", root, tr)
        io.send()

        self.assertEqual(expect_label, latest_name)
        self.assertIsNotNone(latest_case)
        inputs = latest_case.inputs
        self.assertEqual(1, len(inputs))
        outputs = latest_case.outputs
        self.assertEqual(1, len(outputs))

        self.assertTrue("treeusage" in inputs)
        self.assertEqual(testify.NTREE, inputs["treeusage"].dtype)
        ts = testify.Tree()
        inputs["treeusage"].data.Unpack(ts)
        self.assertEqual(root, ts.root)
        self.assertGraphEqual(tr, ts.graph)

        self.assertTrue("stdout" in outputs)
        self.assertEqual(testify.NTREE, outputs["stdout"].dtype)
        outt = testify.Tree()
        outputs["stdout"].data.Unpack(outt)
        self.assertEqual(root, outt.root)
        self.assertGraphEqual(tr, outt.graph)

    def test_graph(self):
        expect_label = "graph"
        io = GenIO(expect_label)
        gr = io.get_graph("graphusage", 43)

        io.set_graph("stdout", gr)
        io.send()

        self.assertEqual(expect_label, latest_name)
        self.assertIsNotNone(latest_case)
        inputs = latest_case.inputs
        self.assertEqual(1, len(inputs))
        outputs = latest_case.outputs
        self.assertEqual(1, len(outputs))

        self.assertTrue("graphusage" in inputs)
        self.assertEqual(testify.GRAPH, inputs["graphusage"].dtype)
        gs = testify.Graph()
        inputs["graphusage"].data.Unpack(gs)
        self.assertGraphEqual(gr, gs)

        self.assertTrue("stdout" in outputs)
        self.assertEqual(testify.GRAPH, outputs["stdout"].dtype)
        outg = testify.Graph()
        outputs["stdout"].data.Unpack(outg)
        self.assertGraphEqual(gr, outg)

    def test_cgraph(self):
        expect_label = "cgraph"
        io = GenIO(expect_label)
        cg = io.get_cgraph("cgraphusage", 47)

        io.set_graph("stdout", cg)
        io.send()

        self.assertEqual(expect_label, latest_name)
        self.assertIsNotNone(latest_case)
        inputs = latest_case.inputs
        self.assertEqual(1, len(inputs))
        outputs = latest_case.outputs
        self.assertEqual(1, len(outputs))

        self.assertTrue("cgraphusage" in inputs)
        self.assertEqual(testify.GRAPH, inputs["cgraphusage"].dtype)
        gs = testify.Graph()
        inputs["cgraphusage"].data.Unpack(gs)
        self.assertGraphEqual(cg, gs)

        self.assertTrue("stdout" in outputs)
        self.assertEqual(testify.GRAPH, outputs["stdout"].dtype)
        outg = testify.Graph()
        outputs["stdout"].data.Unpack(outg)
        self.assertGraphEqual(cg, outg)

if __name__ == "__main__":
    np.random.seed(int(time.time()))
    serve()
    assert(_SERVER is not None)
    client.init("0.0.0.0:" + str(SERVER_PORT))
    unittest.main()
