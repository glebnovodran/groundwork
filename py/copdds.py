import os
import sys
import hou
import imp
import struct
import inspect

try: xrange
except: xrange = range

def getExecPath():
	return os.path.abspath(inspect.getframeinfo(inspect.currentframe()).filename)

def getExecDir():
	return os.path.dirname(getExecPath())

def dbgmsg(msg):
	sys.stdout.write(str(msg) + "\n")

def saveDDS(path, cop):
	dfmt = hou.imageDepth.Float32
	esize = 4
	tfmt = 0x74
	w = cop.xRes()
	h = cop.yRes()
	c = cop.allPixelsAsString(plane="C", depth=dfmt)
	if "A" in cop.planes():
		a = cop.allPixelsAsString(plane="A", depth=dfmt)
	else:
		a = ""
		for i in xrange(w*h):
			a += struct.pack("f", 1.0)
	f = open(path, "wb")
	if not f: return
	f.write(struct.pack("4siiiii", "DDS ", 0x7C, 0x081007, h, w, w*h*esize*4))
	for i in xrange(13): f.write(struct.pack("i", 0))
	f.write(struct.pack("iii", 0x20, 4, tfmt))
	for i in xrange(5): f.write(struct.pack("i", 0))
	f.write(struct.pack("i", 0x1000))
	for i in xrange(4): f.write(struct.pack("i", 0))
	for y in xrange(h):
		idx = (h - 1 - y) * w
		lc = c[idx*esize*3 : (idx + w) * esize*3]
		la = a[idx*esize : (idx + w) * esize]
		for x in xrange(w):
			rgba = lc[x*esize*3 : (x + 1) * esize*3]
			rgba += la[x*esize : (x + 1) * esize]
			f.write(rgba)
	f.close()



