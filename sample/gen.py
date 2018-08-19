''' Generate samples '''

from retrop.generate import GenIO
import retrop.client as client

client.init("0.0.0.0:8581")

sample1 = GenIO("sample1")
sample1.get_arr('"-3tg9$sLdE=d6de', float, 23, (-12, 192.2))
sample1.set_arr([131, 503, 26, 97, 101], int)

sample2 = GenIO("sample2")
sample2.get_arr('dR-mD7wEGCWXZ`bm', float, 41, (-101, 92.2))
sample2.set_arr([11, 3, 21.6, 9, 70.1], float)

sample3 = GenIO("sample3")
sample3.get_arr('\XQ;.C8N7RTy8[f"', int, 71, (-12, 192))
sample3.get_arr('b&g:ZP5-2r)6wu?Y', int, 61, (-101, 92))
sample3.set_arr([1.3, 1, 50.3, 2697, 101], float)

sample4 = GenIO("sample4")
sample4.get_arr('U&A3vhz?2Sb=Durn', float, 142, (-12, 223.2))
sample4.set_arr([77, 523, 6, 729, 50], int)

client.stop()
