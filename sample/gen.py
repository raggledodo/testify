''' Generate samples '''

from retrop.generate import GenIO
import retrop.client as client

cert = open("certs/server.crt", 'rb').read()

client.init("localhost:10000", cert)

sample1 = GenIO("sample1")
sample1.get_arr('"-3tg9$sLdE=d6de', float, 23, (-12, 192.2))
sample1.set_arr('stdout', [131, 503, 26, 97, 101], int)
sample1.send()

sample2 = GenIO("sample2")
sample2.get_arr('dR-mD7wEGCWXZ`bm', float, 41, (-101, 92.2))
sample2.set_arr('stdout', [11, 3, 21.6, 9, 70.1], float)
sample2.send()

sample3 = GenIO("sample3")
sample3.get_arr('\XQ;.C8N7RTy8[f"', int, 71, (-12, 192))
sample3.get_arr('b&g:ZP5-2r)6wu?Y', int, 61, (-101, 92))
sample3.set_arr('stdout', [1.3, 1, 50.3, 2697, 101], float)
sample3.send()

sample4 = GenIO("sample4")
sample4.get_arr('U&A3vhz?2Sb=Durn', float, 142, (-12, 223.2))
sample4.set_arr('stdout', [77, 523, 6, 729, 50], int)
sample4.send()

client.stop()
