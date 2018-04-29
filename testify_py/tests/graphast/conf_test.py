''' Test structure conf '''

import unittest

from graphast.conf import getOpts

class TestStructConf(unittest.TestCase):
	def test_conf(self):
		opts = getOpts("structure.yml")
		classes = opts["classes"]
		self.assertTrue(isinstance(classes, list))
		expectedKeys = ["class", "shapeclass", "args", "names"]
		for cs in classes:
			self.assertTrue(set(expectedKeys) & set(cs.keys()))
