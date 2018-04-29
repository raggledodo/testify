''' Test random equation ast '''

import unittest

import graphast.gen as gen
import graphast.nodes as nodes
from graphast.conf import getOpts

class TestAst(unittest.TestCase):
	def test_rand(self):
		for i in range(1000):
			emindepth = 2
			emaxdepth = 5
			badmap = getOpts("structure.yml")["badchildren"]
			g = gen.generator("structure.yml", emindepth, emaxdepth)
			root = g.generate(retry=100)
			depth = 0
			self.assertEqual(0, root.depth)
			stacks = [root]
			while len(stacks) > 0:
				iter = stacks.pop()
				for arg in iter.args:
					if isinstance(arg, nodes.node):
						self.assertEqual(iter.depth+1, arg.depth)
						if arg.name in badmap:
							nogos = badmap[arg.name]
						else:
							nogos = []
						for bad in nogos:
							self.assertFalse(bad in arg.args)
						stacks.append(arg)
					if isinstance(arg, nodes.leaf):
						self.assertGreater(len(arg.shape), 0)
						self.assertLessEqual(len(arg.shape), 7)
				depth = iter.depth
			# self.assertTrue(emindepth <= depth and depth <= emaxdepth)

	def test_all(self):
		ops = gen.allOps("structure.yml")
		for opname in ops:
			tops = ops[opname]
			for res in tops:
				self.assertEqual(opname, res.name)
				for arg in res.args:
					self.assertTrue(isinstance(arg, nodes.scalar) or isinstance(arg, nodes.leaf))
					if isinstance(arg, nodes.leaf):
						self.assertGreater(len(arg.shape), 0)
						self.assertLessEqual(len(arg.shape), 7)
