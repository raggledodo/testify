'''  Gen supplies random generation options for graph ast '''

import os
import math
import random
import re
import numpy as np
from functools import reduce

import graphast.conf as conf
import graphast.nodes as nodes

MIN_RANK = 0
MAX_RANK = 6

scalarPattern = re.compile(r'scalar\((.*)\)')

class template:
	def __init__(self, gen, sname, atypes, nogo, properties):
		self.gen = gen
		self.sname = sname
		self.atypes = atypes
		self.nogo = nogo
		self.properties = properties

	def getNexts(self, iter):
		if iter.depth < self.gen.mindepth:
			prTerm = 0.0
		else:
			prTerm = math.sqrt(float(iter.depth - self.gen.mindepth) / self.gen.depthdiff)
		atypes = random.choice(self.atypes)
		for argtype in atypes:
			scalarM = scalarPattern.search(argtype)
			if scalarM:
				dtype = scalarM.group(1)
				next = nodes.scalar(dtype)
			elif argtype == "tensor":
				if random.uniform(0, 1) < prTerm:
					next = nodes.leaf()
				else: # select operation
					selection = list(self.gen.opMap.keys())
					for nog in self.nogo:
						selection.remove(nog)
					opname = random.choice(selection)
					next = nodes.node(opname, self.gen.opMap[opname].sname, depth=iter.depth+1)
			else:
				raise Exception("argtype %s not supported" % argtype)
			iter.args.append(next)

class generator:
	def __init__(self, yConf, mindepth, maxdepth):
		assert(mindepth < maxdepth)
		self.mindepth = mindepth
		self.depthdiff = maxdepth - mindepth
		self.opMap = {}
		for opname, sname, atypes, nogo, properties in parseConfig(yConf):
			self.opMap[opname] = template(self, sname, atypes, nogo, properties)

	def mustGenerate(self):
		rootOp = random.choice(list(self.opMap.keys()))
		root = nodes.node(rootOp, self.opMap[rootOp].sname)
		stack = [root]
		while len(stack) > 0:
			iter = stack.pop()
			if isinstance(iter, nodes.node):
				self.opMap[iter.name].getNexts(iter)
				iter.properties = self.opMap[iter.name].properties
				stack.extend(iter.args)
		_shape_populate(root)
		return root
	
	def generate(self, retry = 10):
		for i in range(retry-1):
			try:
				root = self.mustGenerate()
			except:
				continue
			return root
		return self.mustGenerate()

def parseConfig(yConf):
	structs = conf.getOpts(yConf)
	badChildren = structs['badchildren']
	opMap = []
	for opClass in structs["classes"]:
		if "properties" in opClass:
			properties = opClass["properties"]
		else:
			properties = []
		for opname in opClass['names']:
			if isinstance(opClass['args'], list):
				atypes = [[argt.strip() for argt in argstr.split(",")] for argstr in opClass['args']]
			else:
				atypes = [[argt.strip() for argt in opClass['args'].split(",")]]
			if opname in badChildren:
				nogo = badChildren[opname]
			else:
				nogo = []
			opMap.append((opname, opClass['shapeclass'], atypes, nogo, properties))
	return opMap

def allOps(yConf):
	opMap = parseConfig(yConf)
	out = {}
	for opname, sname, args, nogo, properties in opMap:
		out[opname] = []
		for atypes in args:
			res = nodes.node(opname, sname, properties=properties)
			for argtype in atypes:
				scalarM = scalarPattern.search(argtype)
				if scalarM:
					dtype = scalarM.group(1)
					next = nodes.scalar(dtype)
				elif argtype == "tensor":
					next = nodes.leaf()
				else:
					raise Exception("argtype %s not supported" % argtype)
				res.args.append(next)
			_shape_populate(res)
			out[opname].append(res)
	return out

def _shape_populate(root):
	# go down to the leaves and validate
	eps = []
	minrank, maxrank = _validate(root, eps)
	_shape_helper(root, (MIN_RANK, MAX_RANK))
	for ep in eps:
		_shape_helper(ep[0], ep[1])

def _shape_helper(root, limits):
	minr = limits[0]
	maxr = limits[1]
	if maxr == 0:
		root.make_shape([1])
	elif maxr == 1:
		root.make_shape([random.randint(2, 9)])
	else:
		root_shape = list(np.random.randint(2, high=9, size=random.randint(minr, maxr)))
		if len(root_shape) == 0:
			root_shape = [1]
		root.make_shape(root_shape)

def _validate(root, eps):
	minrank = MIN_RANK
	maxrank = MAX_RANK
	if isinstance(root, nodes.node):
		rs = list(map(lambda arg: (arg, _validate(arg, eps)), root.args))

		minrank = reduce(lambda r1, r2: max(r1, r2), 
			map(lambda r: r[1][0], rs), minrank)

		maxrank = reduce(lambda r1, r2: min(r1, r2), 
			map(lambda r: r[1][1], rs), maxrank)

		if maxrank == 0:
			if "rejectscalar" in root.properties:
				raise Exception("invalid root, shape mismatch")

		if "rankscalar" in root.properties:
			maxrank = 0
			eps.extend([(arg, (0, maxrank)) for arg in root.args])
		elif "ranklower" in root.properties:
			if maxrank == 1:
				eps.extend([(arg, (1, maxrank)) for arg in root.args])
			if maxrank > 0:
				maxrank = maxrank - 1
			else: # is scalar
				maxrank = 0
		elif "rankupper" in root.properties:
			if maxrank < MAX_RANK:
				maxrank = maxrank + 1
	return minrank, maxrank
