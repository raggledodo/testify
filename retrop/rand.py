''' Generate inputs for validation '''

import sys
import random
import math
import struct

import numpy as np

class Graph:
    def __init__(self, nverts):
        self.nverts = nverts
        self._data = [0] * (nverts * nverts)

    def connect(self, src, dest):
        self._data[self._index(src, dest)] = 1

    def disconnect(self, src, dest):
        self._data[self._index(src, dest)] = 0

    def get(self, src, dest):
        return self._data[self._index(src, dest)]

    def serialize(self):
        nbytes = int(math.ceil(len(self._data) / 8.0))
        chars = [str(d) for d in self._data]
        bs = [chars[i * 8: (i+1) * 8] for i in range(nbytes)]
        bstr = [''.join(b[::-1]) for b in bs]
        ints = [int(b, 2) for b in bstr]
        chars = [struct.pack('B', b) for b in ints]
        return b''.join(chars)

    def breadth_traverse(self, root, cb):
        visited = [False] * self.nverts
        q = [root]

        while len(q) > 0:
            r = q.pop()
            if visited[r]:
                continue
            children = []
            for child in range(self.nverts):
                if self.get(r, child):
                    children.append(child)
            q = children + q
            cb(r, children)
            visited[r] = True

    def _index(self, src, dest):
        out = src * self.nverts + dest
        if out >= len(self._data):
            raise "src=%d,dest=%d out of bound" % (src, dest)
        return out

def get_arr(dtype, size, bounds=None):
    if dtype is int:
        mi = -9223372036854775808
        ma = 9223372036854775807
        if bounds is not None:
            mi = bounds[0]
            ma = bounds[1]
        arr = np.random.randint(mi, ma + 1, size)
    elif dtype is float:
        mi = sys.float_info.min
        ma = sys.float_info.max
        if bounds is not None:
            mi = bounds[0]
            ma = bounds[1]
        arr = (ma - mi) * np.random.random(size) + mi
    else:
        raise "unsupported type: " + str(dtype)
    return list(arr)

def get_str(size, content):
    idcs = get_arr(int, size, (0, len(content)-1))
    return ''.join([content[i] for i in idcs])

def get_tree(nverts):
    out = Graph(nverts)
    if nverts == 0:
        raise "attempting to generate empty tree"
    out.connect(0, 1)
    if nverts > 2:
        parents = get_arr(int, nverts - 2, (0, nverts-1))
        for i, parent in enumerate(parents):
            out.connect(parent % (i + 2), i + 2)
    return 0, out

def _rand_connect(graph):
    N = graph.nverts
    nedges = N * (N - 1)
    conns = get_arr(int, nedges, (0, 1))
    cidx = 0
    for i in range(N):
        for j in range(N):
            if i != j:
                if conns[cidx] > 0:
                    graph.connect(i, j)
                cidx = cidx + 1

def get_graph(nverts):
    out = Graph(nverts)
    _rand_connect(out)
    return out

def get_cgraph(nverts):
    _, out = get_tree(nverts)
    _rand_connect(out)
    return out
