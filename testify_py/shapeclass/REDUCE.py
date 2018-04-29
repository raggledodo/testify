''' Shape propagation for REDUCE class operations '''

import random
import numpy as np

def expand(shape, idx, mul):
	after = shape[idx:]
	out = shape[:idx]
	out.append(mul)
	out.extend(after)
	return out

def REDUCE(parent, makes):
	n = len(parent)
	if len(makes) > 1:
		limit = n+1
		idx = random.randint(0, n)
		mul = random.randint(1, 9)
		makes[0](expand(parent, idx, mul))
		makes[1]([1], str(idx))
	else:
		makes[0](list(np.random.randint(1, high=9, size=random.randint(1, n))))
