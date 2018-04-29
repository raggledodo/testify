''' Shape class output map of functions  '''

import re
import os
import glob
import inspect

registry = {}

def reg_func(func):
	registry[func.__name__] = func

dir_path = os.path.dirname(os.path.realpath(__file__))
pathglob = os.path.join(dir_path, '*.py')
regdef = re.compile('def ([^\(]*)')
regitercmd = 'reg_func(%s)'
for pyfile in glob.glob(pathglob):
	if not os.path.samefile(pyfile, __file__): # prevent infinite exec loop
		with open(pyfile, 'r') as f:
			pydata = f.read()
			exec(pydata)
			defs = regdef.finditer(pydata)
			for d in defs:
				dname = d.group(1)
				exec(regitercmd % dname)
