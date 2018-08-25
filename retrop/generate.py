''' Generate inputs and send output to client '''

import proto.testify_pb2 as testify

import retrop.client as client
import retrop.rand as rand

class GenIO:
    def __init__(self, name):
        self.testname = name
        self.gcase = testify.GeneratedCase()

    def get_arr(self, usage, dtype, size, range=None):
        out = rand.get_arr(dtype, size, range)
        input = self.gcase.inputs.add()
        input.usage = usage
        if dtype is int:
            input.dtype = testify.INT64S
            arr = testify.Int64s()
            arr.data[:] = out
            input.data.Pack(arr)
        elif dtype is float:
            input.dtype = testify.DOUBLES
            arr = testify.Doubles()
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
        input = self.gcase.inputs.add()
        input.usage = usage
        input.dtype = testify.BYTES
        arr = testify.Bytes()
        arr.data = out.encode()
        input.data.Pack(arr)
        return out

    def get_tree(self, usage, nverts):
        root, out = rand.get_tree(nverts)
        assert(isinstance(out, rand.Graph))
        input = self.gcase.inputs.add()
        input.usage = usage
        input.dtype = testify.NTREE
        tree = testify.Tree()
        tree.root = root
        tree.graph.matrix = out.serialize()
        tree.graph.nverts = out.nverts
        input.data.Pack(tree)
        return root, out

    def get_graph(self, usage, nverts):
        out = rand.get_graph(nverts)
        assert(isinstance(out, rand.Graph))
        input = self.gcase.inputs.add()
        input.usage = usage
        input.dtype = testify.GRAPH
        graph = testify.Graph()
        graph.matrix = out.serialize()
        graph.nverts = out.nverts
        input.data.Pack(graph)
        return out

    def get_cgraph(self, usage, nverts):
        out = rand.get_cgraph(nverts)
        assert(isinstance(out, rand.Graph))
        input = self.gcase.inputs.add()
        input.usage = usage
        input.dtype = testify.GRAPH
        graph = testify.Graph()
        graph.matrix = out.serialize()
        graph.nverts = out.nverts
        input.data.Pack(graph)
        return out

    def set_arr(self, usage, arr, dtype):
        output = self.gcase.outputs.add()
        output.usage = usage
        if dtype is int:
            output.dtype = testify.INT64S
            iarr = testify.Int64s()
            iarr.data[:] = arr
            output.data.Pack(iarr)
        elif dtype is float:
            output.dtype = testify.DOUBLES
            darr = testify.Doubles()
            darr.data[:] = arr
            output.data.Pack(darr)

    def set_tree(self, usage, root, graph):
        assert(isinstance(graph, rand.Graph))
        output = self.gcase.outputs.add()
        output.usage = usage
        output.dtype = testify.NTREE
        tree = testify.Tree()
        tree.root = root
        tree.graph.matrix = graph.serialize()
        tree.graph.nverts = graph.nverts
        output.data.Pack(tree)

    def set_graph(self, usage, graph):
        assert(isinstance(graph, rand.Graph))
        output = self.gcase.outputs.add()
        output.usage = usage
        output.dtype = testify.GRAPH
        gr = testify.Graph()
        gr.matrix = graph.serialize()
        gr.nverts = graph.nverts
        output.data.Pack(gr)

    def send(self):
        if len(self.gcase.inputs) < 1 or len(self.gcase.outputs) < 1:
            return
        client.send(self.testname, self.gcase)
