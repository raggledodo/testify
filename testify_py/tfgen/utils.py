''' Variable generating utility '''

import random
import string

def randVariable(n):
	postfix = ''.join(random.choice(string.ascii_uppercase + string.digits) for _ in range(n-1))
	return random.choice(string.ascii_uppercase + string.ascii_lowercase) + postfix
