''' Test retrop rand generator '''

import os
import sys
import unittest
import time

import numpy as np

import retrop.rand as rand

class RandTest(unittest.TestCase):
    def assertArrBetween(self, low, high, arr):
        for e in arr:
            self.assertLessEqual(low, e)
            self.assertGreaterEqual(high, e)

    def _vec_randomness(self, arr, arr2, bounds, N, similarity_thresh):
        # constraint
        self.assertEqual(N, len(arr))
        self.assertArrBetween(bounds[0], bounds[1], arr)

        # randomness between different arrays
        dist = 0
        for i in range(N):
            dist += (arr[i] - arr2[i]) ** 2
        dist = (dist ** 0.5) / (bounds[1] - bounds[0])
        self.assertLess(similarity_thresh, dist)

    def test_ints(self):
        separation_metric = 1
        similarity_thresh = 0.1

        bounds = (-100, 252)
        N = 3000
        iarr = rand.get_arr(int, N, bounds)
        iarr2 = rand.get_arr(int, N, bounds)

        self._vec_randomness(iarr, iarr2, bounds, N, similarity_thresh)

        # randomness within a single array
        iarr.sort()
        diff = 0
        for i in range(N-1):
            diff += (iarr[i+1] - iarr[i])
        avgdiff = diff / (N - 1)
        self.assertGreater(separation_metric, avgdiff)

    def test_floats(self):
        separation_metric = 0.25
        similarity_thresh = 0.1

        bounds = (-235, 252)
        N = 7000
        darr = rand.get_arr(float, N, bounds)
        darr2 = rand.get_arr(float, N, bounds)

        self._vec_randomness(darr, darr2, bounds, N, similarity_thresh)

        # randomness within a single array
        darr.sort()
        diff = 0
        for i in range(N-1):
            diff += (darr[i+1] - darr[i])
        avgdiff = diff / (N - 1)
        self.assertGreater(separation_metric, avgdiff)

    def test_str(self):
        similarity_thresh = 0.1

        content = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ" +\
            "abcdefghijklmnopqrstuvwxyz"
        bounds = (ord('0'), ord('z'))
        N = 19
        s = rand.get_str(N, content=content)
        s2 = rand.get_str(N, content=content)

        self._vec_randomness([ord(c) for c in s], [ord(c) for c in s2],
            bounds, N, similarity_thresh)

    def test_tree(self):
        N = 147
        root, tr = rand.get_tree(N)
        _, tr2 = rand.get_tree(N)

        # constraint
        visited = [False] * N
        def cb(node, children):
            self.assertFalse(visited[node])
            visited[node] = True
        tr.breadth_traverse(root, cb)
        for v in visited:
            self.assertTrue(v)

        # randomness between trees
        nand = 0
        nor = 0
        for i in range(N):
            for j in range(N):
                nand += tr.get(i, j) and tr2.get(i, j)
                nor += tr.get(i, j) or tr2.get(i, j)
        sim_frac = nand / nor
        self.assertGreater(0.1, sim_frac)

    def test_graph(self):
        N = 143
        gr = rand.get_graph(N)
        gr2 = rand.get_graph(N)

        # randomness between graph
        nand = 0
        nor = 0
        for i in range(N):
            for j in range(N):
                nand += gr.get(i, j) and gr2.get(i, j)
                nor += gr.get(i, j) or gr2.get(i, j)
        sim_frac = nand / nor
        self.assertGreater(0.5, sim_frac)

    def test_cgraph(self):
        N = 147
        cg = rand.get_cgraph(N)
        cg2 = rand.get_cgraph(N)

        # constraint
        visited = [False] * N
        def cb(node, children):
            visited[node] = True
        cg.breadth_traverse(0, cb)
        for v in visited:
            self.assertTrue(v)

        # randomness between graph
        nand = 0
        nor = 0
        for i in range(N):
            for j in range(N):
                nand += cg.get(i, j) and cg2.get(i, j)
                nor += cg.get(i, j) or cg2.get(i, j)
        sim_frac = nand / nor
        self.assertGreater(0.5, sim_frac)

def get_envar(key, dtype=int, default=None):
    try:
        return dtype(os.environ[key])
    except:
        return default

if __name__ == "__main__":
    np.random.seed(int(time.time()))
    repeat = get_envar('PYTEST_REPEAT', default=1)
    for iteration in range(repeat):
        success = unittest.main(exit=False).result.wasSuccessful()
        if not success:
            sys.exit(1)
