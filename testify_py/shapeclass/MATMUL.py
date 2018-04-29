''' Shape propagation for MATMUL class operations '''

import random

def MATMUL(parent, makes):
	assert len(makes) == 2
	if len(parent) < 2:
		parent = [parent[0], 1]
	common = random.randint(1, 9)
	if len(parent) > 2:
		beyond = parent[:-2]
	else:
		beyond = []
	makes[0](beyond + [parent[-2], common])
	makes[1](beyond + [common, parent[-1]])
