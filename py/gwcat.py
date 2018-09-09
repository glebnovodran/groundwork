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

def writeRsrcSig(f, sig, ver):
	sig = "rsrc:" + sig
	if len(sig) > 15: sig = sig[:15]
	ver &= 0xFFFF
	f.write(struct.pack("16s", sig))
	f.write(struct.pack("4s", "{:02X}{:02X}".format(ver >> 8, ver & 0xFF)))

def fpatch(f, offs, val):
	pos = f.tell()
	f.seek(offs)
	f.write(struct.pack("I", val))
	f.seek(pos)

def align(x, a):
	return ((x + (a - 1)) // a) * a

class Strings:
	def __init__(self):
		self.data = ""
		self.strToOffs = {}
		self.offs = 0

	def add(self, s):
		if not s in self.strToOffs:
			offs = self.offs
			self.strToOffs[s] = offs
			n = len(s) + 1;
			self.data += struct.pack(str(n)+"s", s)
			self.offs += n
		return self.strToOffs[s]

class GWResKind:
	def __init__(self): pass

GWResKind.MODEL = 1
GWResKind.DDS = 0x100
GWResKind.TDMOT = 0x101
GWResKind.TDGEO = 0x102

class GWResFile:
	def __init__(self, fname, kind):
		self.fname = fname
		self.kind = kind
		self.nameOffs = -1

class GWCatalog:
	def __init__(self):
		self.strs = Strings()
		self.files = []

	def add(self, fname, kind):
		rf = GWResFile(fname, kind)
		rf.nameOffs = self.strs.add(fname)
		self.files.append(rf)

	def write(self, f):
		sig = "GWCatalog"
		ver = 0x100
		writeRsrcSig(f, sig, ver)
		f.write(struct.pack("i", 0)) # +14: file size
		f.write(struct.pack("I", 0)) # +18 -> strs
		f.write(struct.pack("I", len(self.strs.data))) # +1C
		n = len(self.files)
		f.write(struct.pack("i", n)) # +20
		for rf in self.files:
			f.write(struct.pack("i", rf.kind))
			f.write(struct.pack("i", rf.nameOffs))

		fpatch(f, 0x18, f.tell()) # -> strs
		f.write(self.strs.data)

		fpatch(f, 0x14, f.tell()) # +14 := file size

	def save(self, fpath):
		if len(self.files) < 1: return
		try:
			f = open(fpath, "wb")
		except IOError:
			dbgmsg("Can't create output file: " + fpath)
		else:
			self.write(f)
			f.close()
