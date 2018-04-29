'''  Nodes of graph ast '''

import numpy as np

from shapeclass.out import registry

class node:
	def __init__(self, name, sname, depth=0, properties = []):
		self.name = name
		self.depth = depth
		self.args = []
		self.properties = properties
		self.shape = []
		if sname in registry:
			self.shaper = registry[sname]
		else:
			raise Exception("unsupported shape class "+sname)

	def __repr__(self):
		return '{"name": "%s", "args": [%s]}' % (self.name, ', '.join([str(arg) for arg in self.args]))

	def make_shape(self, shape, value=None):
		self.shape = shape
		self.shaper(shape, [arg.make_shape for arg in self.args])

class leaf:
	def __init__(self):
		self.shape = []

	def __repr__(self):
		return '"LEAF('+str(self.shape)+')"'

	def make_shape(self, shape, value=None):
		self.shape = shape

class scalar:
	def __init__(self, dtype):
		if dtype == "double":
			self.value = "random.random()"
		elif dtype == "int":
			self.value = "random.randint(1, 9)"
		else:
			raise Exception("unsupported type "+dtype)

	def __repr__(self):
		return '"SCALAR('+self.value+')"'

	def make_shape(self, shape, value=None):
		if value:
			self.value = value

# bottom up traverse utility
def traverse(root, func):
	collection = []
	deps = []
	if isinstance(root, node):
		for arg in root.args:
			depid, coll = traverse(arg, func)
			collection.extend(coll)
			deps.append(depid)
	id, info = func(root, deps)
	collection.append(info)
	return id, collection
