#!/usr/bin/python

import spi

ret = spi.openSPI(speed=1000000)
print "openSPI returns: ", ret
fd = ret ["fd"]
print "fd = ", fd

print "Reading nRF24L01 status registers:"

for x in range(28):
	dat = spi.transfer(fd, (x, 0))
	print "nRF Register 0x%X: %X" % (x, dat[1])

spi.closeSPI (fd)
