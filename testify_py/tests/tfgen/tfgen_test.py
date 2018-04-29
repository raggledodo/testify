''' Test and run tfgen '''

import os
import string
import random
import unittest

from graphast.gen import allOps, generator
from graphast.nodes import traverse
from tfgen.tfgen import tf_gen
from tfgen.utils import randVariable

MINDEPTH = os.environ['MINDEPTH'] if 'MINDEPTH' in os.environ else 1 
MAXDEPTH = os.environ['MAXDEPTH'] if 'MAXDEPTH' in os.environ else 10

def treesize(root):
	def count(_, deps):
		return sum(deps) + 1, None
	counter, _ = traverse(root, count)
	return counter

def randarr(n):
	return [randVariable(16) for _ in range(n)]

class TestTfgen(unittest.TestCase):
	def test_detgen(self):
		ops = allOps("structure.yml")
		for opname in ops: 
			tops = ops[opname]
			for root in tops:
				createorder = randarr(treesize(root))
				script = tf_gen(root, createorder)
				try:
					exec(script)
				except Exception as e:
					print(script)
					raise e

	def test_randgen(self):
		rgen = generator("structure.yml", MINDEPTH, MAXDEPTH) 
		for i in range(100):
			root = rgen.generate(retry=100)
			createorder = randarr(treesize(root))
			script = tf_gen(root, createorder)
			try:
				exec(script)
			except Exception as e:
				print(script)
				raise e

if __name__ == "__main__":
	unittest.main()
