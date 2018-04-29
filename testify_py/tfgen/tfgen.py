""" Serializing ast graph as a tensorflow script """

import random
import string

import graphast.nodes as nodes

# file 'save_data.py' defined at runtime
TF_SCRIPT = '''import os
import random
import tensorflow as tf

LOG_RUNTIME = 'LOG_RUNTIME' in os.environ

{0}
{1}

init = tf.global_variables_initializer()
outs = []

with tf.Session() as sess:
	sess.run(init)

	if LOG_RUNTIME:
		print("adding random scalar results")
	placemap = {2}
	for label in placemap:
		place = placemap[label]
		outs.append(("place", label, place))

	if LOG_RUNTIME:
		print("adding variable results")
	varmap = {3}
	for label in varmap:
		input = varmap[label]
		res = sess.run(input)
		res = res.astype(float)
		outs.append(("variable", label, res))

	if LOG_RUNTIME:
		print("adding gradient results")
	gradmap = {4}
	for label in gradmap:
		gradres = gradmap[label]
		res = sess.run(gradres)
		res = res.astype(float)
		outs.append(("gradient", label, res))

	if LOG_RUNTIME:
		print("adding output results")
	outs.append(("output", "{5}", sess.run({5}).astype(float)))
'''

class declarable:
	def __init__(self, createOrder):
		self.createOrder = createOrder
		self.leaves = []
		self.placescalars = []
		self.i = 0

	def nextId(self):
		id = self.createOrder[self.i]
		self.i = self.i + 1
		return id

	def declare(self, node, deps):
		id = self.nextId()
		if isinstance(node, nodes.node):
			funcname = node.name.lower()
			if "rmax" == funcname:
				funcname = "reduce_max"
			elif "rsum" == funcname:
				funcname = "reduce_sum"
			elif "neg" == funcname:
				funcname = "negative"
			elif "sub" == funcname:
				funcname = "subtract"
			elif "mul" == funcname:
				funcname = "multiply"
			decl = "tf.%s(%s)" % (funcname, ', '.join(deps))
		elif isinstance(node, nodes.leaf):
			decl = "tf.Variable(tf.random_uniform(%s))" % str(node.shape)
			self.leaves.append(id)
		elif isinstance(node, nodes.scalar):
			decl = node.value
			try:
				int(node.value)
			except ValueError:
				self.placescalars.append(id)
		else:
			raise Exception("unsupported node type")
		return id, "%s = %s" % (id, decl)

def tf_gen(root, createOrder, script_prefix = "", script_postfix = ""):
	decl = declarable(createOrder)
	id, lines = nodes.traverse(root, decl.declare)
	grads = ["grad_" + leaf for leaf in decl.leaves]
	if len(decl.leaves) > 1:
		tfGrad = "%s = tf.gradients(%s, [%s])" % \
			(', '.join(grads), id, ', '.join(decl.leaves))
	else:
		tfGrad = "%s = tf.gradients(%s, %s)[0]" % \
			(', '.join(grads), id, decl.leaves[0])

	placeMap = ', '.join([ '"{0}": {0}'.format(place) for place in decl.placescalars])
	leafMap = ', '.join([ '"{0}": {0}'.format(leaf) for leaf in decl.leaves])
	gradMap = ', '.join([ '"{0}": grad_{0}'.format(leaf) for leaf in decl.leaves])

	script = (script_prefix + TF_SCRIPT + script_postfix).format(
		'\n'.join(lines),
		tfGrad,
		"{" + placeMap + "}",
		"{" + leafMap + "}",
		"{" + gradMap + "}",
		id)
	return script
