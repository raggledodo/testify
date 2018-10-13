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
        input = self.gcase.inputs[usage]
        if dtype is int:
            input.dint64s.data[:] = out
        elif dtype is float:
            input.ddoubles.data[:] = out
        else:
            raise "unsupported type: " + str(dtype)
        return out

    def get_str(self, usage, size,
        content="0123456789!@#$%^&*" +\
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ" +\
            "abcdefghijklmnopqrstuvwxyz"):
        out = rand.get_str(size, content)
        input = self.gcase.inputs[usage]
        input.dbytes.data = out.encode()
        return out

    def get_tree(self, usage, nverts):
        root, out = rand.get_tree(nverts)
        assert(isinstance(out, rand.Graph))
        input = self.gcase.inputs[usage]
        input.dtree.root = root
        input.dtree.graph.matrix = out.serialize()
        input.dtree.graph.nverts = out.nverts
        return root, out

    def get_graph(self, usage, nverts):
        out = rand.get_graph(nverts)
        assert(isinstance(out, rand.Graph))
        input = self.gcase.inputs[usage]
        input.dgraph.matrix = out.serialize()
        input.dgraph.nverts = out.nverts
        return out

    def get_cgraph(self, usage, nverts):
        out = rand.get_cgraph(nverts)
        assert(isinstance(out, rand.Graph))
        input = self.gcase.inputs[usage]
        input.dgraph.matrix = out.serialize()
        input.dgraph.nverts = out.nverts
        return out

    def set_arr(self, usage, arr, dtype):
        output = self.gcase.outputs[usage]
        if dtype is int:
            output.dint64s.data[:] = arr
        elif dtype is float:
            output.ddoubles.data[:] = arr

    def set_tree(self, usage, root, graph):
        assert(isinstance(graph, rand.Graph))
        output = self.gcase.outputs[usage]
        output.dtree.root = root
        output.dtree.graph.matrix = graph.serialize()
        output.dtree.graph.nverts = graph.nverts

    def set_graph(self, usage, graph):
        assert(isinstance(graph, rand.Graph))
        output = self.gcase.outputs[usage]
        output.dgraph.matrix = graph.serialize()
        output.dgraph.nverts = graph.nverts

    def can_send(self):
        return len(self.gcase.inputs) > 0

    def send(self):
        if len(self.gcase.inputs) < 1:
            raise "sending empty case"
        client.send(self.testname, self.gcase)
