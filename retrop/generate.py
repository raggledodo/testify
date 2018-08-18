''' Generate inputs and send output to client '''

import proto.testdata_pb2 as testdata
import proto.record_pb2 as record

import client
import rand

class GenIO:
    def __init__(self, name):
        self.testname = name
        self.output = testdata.TestOutput()

    def get_arr(self, usage, dtype, size, range=None):
        out = rand.get_arr(dtype, size, range)
        input = self.output.inputs.add()
        input.usage = usage
        if dtype is int:
            input.dtype = testdata.INT64S
            arr = testdata.Int64s()
            arr.data[:] = out
            input.data.Pack(arr)
        elif dtype is float:
            input.dtype = testdata.DOUBLES
            arr = testdata.Doubles()
            arr.data[:] = out
            input.data.Pack(arr)
        else:
            raise "unsupported type: " + str(dtype)
        return out

    def get_str(self, usage, size,
        content="0123456789!@#$%^&*" +\
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ" +\
            "abcdefghijklmnopqrstuvwxyz"):
        out = rand.get_str(size, content)
        input = self.output.inputs.add()
        input.usage = usage
        input.dtype = testdata.BYTES
        arr = testdata.Bytes()
        arr.data = out.encode()
        input.data.Pack(arr)
        return out

    def get_tree(self, usage, nverts):
        root, out = rand.get_tree(nverts)
        assert(isinstance(out, rand.Graph))
        input = self.output.inputs.add()
        input.usage = usage
        input.dtype = testdata.NTREE
        tree = testdata.Tree()
        tree.root = root
        tree.graph.matrix = out.serialize()
        tree.graph.nverts = out.nverts
        input.data.Pack(tree)
        return root, out

    def get_graph(self, usage, nverts):
        out = rand.get_graph(nverts)
        assert(isinstance(out, rand.Graph))
        input = self.output.inputs.add()
        input.usage = usage
        input.dtype = testdata.GRAPH
        graph = testdata.Graph()
        graph.matrix = out.serialize()
        graph.nverts = out.nverts
        input.data.Pack(graph)
        return out

    def get_cgraph(self, usage, nverts):
        out = rand.get_cgraph(nverts)
        assert(isinstance(out, rand.Graph))
        input = self.output.inputs.add()
        input.usage = usage
        input.dtype = testdata.GRAPH
        graph = testdata.Graph()
        graph.matrix = out.serialize()
        graph.nverts = out.nverts
        input.data.Pack(graph)
        return out

    def set_arr(self, arr, dtype):
        if dtype is int:
            self.output.dtype = testdata.INT64S
            iarr = testdata.Int64s()
            iarr.data[:] = arr
            self.output.output.Pack(iarr)
        elif dtype is float:
            self.output.dtype = testdata.DOUBLES
            darr = testdata.Doubles()
            darr.data[:] = arr
            self.output.output.Pack(darr)
        self.send()

    def set_tree(self, root, graph):
        assert(isinstance(graph, rand.Graph))
        self.output.dtype = testdata.NTREE
        tree = testdata.Tree()
        tree.root = root
        tree.graph.matrix = graph.serialize()
        tree.graph.nverts = graph.nverts
        self.output.output.Pack(tree)
        self.send()

    def set_graph(self, graph):
        assert(isinstance(graph, rand.Graph))
        self.output.dtype = testdata.GRAPH
        gr = testdata.Graph()
        gr.matrix = graph.serialize()
        gr.nverts = graph.nverts
        self.output.output.Pack(gr)
        self.send()

    def send(self):
        if len(self.output.inputs) < 1:
            return
        client.send(self.testname, self.output)
