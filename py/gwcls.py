import os
import sys
import hou
import imp
import struct
import inspect
from math import *

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
			n = len(s) + 1
			self.data += struct.pack(str(n)+"s", s)
			self.offs += n
		return self.strToOffs[s]


def calcTriNrm(i0, i1, i2, pnts):
	v0 = pnts[i0].position()
	v1 = pnts[i1].position()
	v2 = pnts[i2].position()
	nrm = (v0 - v1).cross(v2 - v1)
	return nrm.normalized()

def calcPolNrm(pntIdxLst, pnts):
	n = len(pntIdxLst)
	nrm = [0.0 for i in xrange(3)]
	for i in xrange(n):
		j = i - 1
		if j < 0: j = n - 1
		ith = pnts[pntIdxLst[i]].position()
		jth = pnts[pntIdxLst[j]].position()
		d = [ith[k] - jth[k] for k in xrange(3)]
		s = [ith[k] + jth[k] for k in xrange(3)]
		p = [d[1]*s[2], d[2]*s[0], d[0]*s[1]]
		for k in xrange(3): nrm[k] += p[k]
	norm = 0.0
	for k in xrange(3): norm += nrm[k]*nrm[k]
	norm = sqrt(norm)
	if norm > 0.0:
		nr = 1.0 / norm
		for k in xrange(3): nrm[k] *= nr
	return hou.Vector3(nrm)

def pntInTri(ipnt, i0, i1, i2, pnts):
	pnt = pnts[ipnt].position()
	v0 = pnts[i0].position()
	v1 = pnts[i1].position()
	v2 = pnts[i2].position()
	a = v0 - pnt
	b = v1 - pnt
	c = v2 - pnt
	u = b.cross(c)
	if u.dot(c.cross(a)) < 0.0: return False
	if u.dot(a.cross(b)) < 0.0: return False
	return True

class Poly:
	def __init__(self, cls, prim):
		self.cls = cls
		self.prim = prim
		self.pnts = []
		self.idxOffs = -1
		self.triOffs = -1
		for vtx in prim.vertices():
			self.pnts.append(vtx.point().number())
		nvtx = len(self.pnts)
		if nvtx == 3:
			self.nrm = calcTriNrm(self.pnts[0], self.pnts[1], self.pnts[2], self.cls.pnts)
		else:
			self.nrm = calcPolNrm(self.pnts, self.cls.pnts)
		p0 = self.cls.pnts[self.pnts[0]].position()
		self.bboxMin = [p0[i] for i in xrange(3)]
		self.bboxMax = [p0[i] for i in xrange(3)]
		for i in xrange(1, nvtx):
			p = self.cls.pnts[self.pnts[i]].position()
			for j in xrange(3):
				self.bboxMin[j] = min(self.bboxMin[j], p[j])
				self.bboxMax[j] = max(self.bboxMax[j], p[j])

	def triangulate(self):
		self.tris = None
		n = len(self.pnts)
		if n < 4: return
		self.tris = []
		lnk = []
		for i in xrange(n):
			prev = -1
			next = -1
			if i > 0: prev = i - 1
			else: prev = n - 1
			if i < n - 1: next = i + 1
			else: next = 0
			lnk.append([prev, next])
		cnt = n
		idx = 0
		while cnt > 3:
			prev = lnk[idx][0]
			next = lnk[idx][1]
			i0 = self.pnts[prev]
			i1 = self.pnts[idx]
			i2 = self.pnts[next]
			triNrm = calcTriNrm(i0, i1, i2, self.cls.pnts)
			d = triNrm.dot(self.nrm)
			flg = d >= 0.0
			if flg:
				next = lnk[next][1]
				while True:
					if pntInTri(next, i0, i1, i2, self.cls.pnts):
						flg = False
						break
					next = lnk[next][1]
					if next == prev: break
			if flg:
				self.tris.append(lnk[idx][0])
				self.tris.append(idx)
				self.tris.append(lnk[idx][1])
				lnk[lnk[idx][0]][1] = lnk[idx][1]
				lnk[lnk[idx][1]][0] = lnk[idx][0]
				cnt -= 1
				idx = lnk[idx][0]
			else:
				idx = lnk[idx][1]
		self.tris.append(lnk[idx][0])
		self.tris.append(idx)
		self.tris.append(lnk[idx][1])

	def getBBoxCenter(self):
		return (hou.Vector3(self.bboxMin) + hou.Vector3(self.bboxMax)) * 0.5

	def write(self, f):
		f.write(struct.pack("ffffff",
			self.bboxMin[0], self.bboxMin[1], self.bboxMin[2],
			self.bboxMax[0], self.bboxMax[1], self.bboxMax[2]))
		f.write(struct.pack("fff", self.nrm[0], self.nrm[1], self.nrm[2]))
		f.write(struct.pack("iii", self.idxOffs, len(self.pnts), self.triOffs))

class BVHNode:
	def __init__(self, bvh):
		self.bvh = bvh
		self.pols = []
		self.left = None
		self.right = None
		self.bvh.addNode(self)

	def addPols(self, idx, cnt):
		for i in xrange(cnt):
			self.pols.append(self.bvh.pols[idx + i])
		self.bboxFromPols(idx, cnt)

	def bboxFromPols(self, idx, cnt):
		pol = self.bvh.cls.pols[self.bvh.pols[idx]]
		self.bboxMin = [pol.bboxMin[i] for i in xrange(3)]
		self.bboxMax = [pol.bboxMax[i] for i in xrange(3)]
		for i in xrange(1, cnt):
			pol = self.bvh.cls.pols[self.bvh.pols[idx + i]]
			for j in xrange(3):
				self.bboxMin[j] = min(self.bboxMin[j], pol.bboxMin[j])
				self.bboxMax[j] = max(self.bboxMax[j], pol.bboxMax[j])

	def build(self, idx, cnt, axis):
		if cnt == 1:
			self.addPols(idx, cnt)
		elif cnt == 2:
			self.left = BVHNode(self.bvh)
			self.left.addPols(idx, 1)
			self.right = BVHNode(self.bvh)
			self.right.addPols(idx+1, 1)
			self.bboxMin = [min(self.left.bboxMin[i], self.right.bboxMin[i]) for i in xrange(3)]
			self.bboxMax = [max(self.left.bboxMax[i], self.right.bboxMax[i]) for i in xrange(3)]
		else:
			self.bboxFromPols(idx, cnt)
			pivot = (self.bboxMin[axis] + self.bboxMax[axis]) * 0.5
			mid = self.bvh.split(idx, cnt, pivot, axis)
			nextAxis = (axis + 1) % 3
			self.left = BVHNode(self.bvh)
			self.right = BVHNode(self.bvh)
			self.left.build(idx, mid, nextAxis)
			self.right.build(idx + mid, cnt - mid, nextAxis)

	def write(self, f):
		f.write(struct.pack("ffffff",
			self.bboxMin[0], self.bboxMin[1], self.bboxMin[2],
			self.bboxMax[0], self.bboxMax[1], self.bboxMax[2]))
		polId = -1
		left = -1
		right = -1
		if len(self.pols) > 0: polId = self.pols[0]
		if self.left: left = self.bvh.nodeMap[self.left]
		if self.right: right = self.bvh.nodeMap[self.right]
		if right < 0:
			f.write(struct.pack("i", polId))
		else:
			f.write(struct.pack("i", left))
		f.write(struct.pack("i", right))

class BVH:
	def __init__(self, cls):
		self.cls = cls
		npol = len(self.cls.pols)
		self.pols = [i for i in xrange(npol)]
		self.nodeLst = []
		self.nodeMap = {}
		self.root = BVHNode(self)
		bbsize = [(self.cls.bboxMin[i] + self.cls.bboxMax[i]) * 0.5 for i in xrange(3)]
		bbr = max(bbsize)
		axis = 0
		if bbr == bbsize[1]: axis = 1
		elif bbr == bbsize[2]: axis = 2
		self.root.build(0, npol, axis)

	def addNode(self, node):
		self.nodeMap[node] = len(self.nodeLst)
		self.nodeLst.append(node)

	def split(self, idx, cnt, pivot, axis):
		mid = 0
		for i in xrange(cnt):
			cent = self.cls.pols[self.pols[idx + i]].getBBoxCenter()[axis]
			if cent < pivot:
				t = self.pols[idx + i]
				self.pols[idx + i] = self.pols[idx + mid]
				self.pols[idx + mid] = t
				mid += 1
		if mid == 0 or mid == cnt: mid = cnt / 2
		return mid

	def write(self, f):
		for node in self.nodeLst:
			node.write(f)

class GWCollisionResource:
	def __init__(self, sop):
		self.sop = sop
		self.geo = sop.geometry()
		self.pnts = self.geo.points()
		self.strs = Strings()
		self.pathOffs = self.strs.add(self.sop.parent().path())
		self.pols = []
		for prim in self.geo.prims():
			if prim.type() == hou.primType.Polygon and len(prim.vertices()) >= 3:
				self.pols.append(Poly(self, prim))
		if len(self.pols) < 1:
			dbgmsg("No collision polys!")
			return
		self.idx = []
		for pol in self.pols:
			pol.idxOffs = len(self.idx)
			for pid in pol.pnts:
				self.idx.append(pid)
		for pol in self.pols:
			pol.triangulate()
		triOffs = 0
		for pol in self.pols:
			if pol.tris:
				pol.triOffs = triOffs
				triOffs += len(pol.tris)
		self.bboxMin = [self.pols[0].bboxMin[i] for i in xrange(3)]
		self.bboxMax = [self.pols[0].bboxMax[i] for i in xrange(3)]
		for i in xrange(1, len(self.pols)):
			for j in xrange(3):
				self.bboxMin[j] = min(self.bboxMin[j], self.pols[i].bboxMin[j])
				self.bboxMax[j] = max(self.bboxMax[j], self.pols[i].bboxMax[j])
		self.bvh = BVH(self)

	def write(self, f):
		sig = "GWCls"
		ver = 0x100
		writeRsrcSig(f, sig, ver)
		f.write(struct.pack("i", 0)) # +14: file size
		f.write(struct.pack("I", 0)) # +18 -> strs
		f.write(struct.pack("I", len(self.strs.data))) # +1C size(strs)
		npnt = len(self.pnts)
		npol = len(self.pols)
		f.write(struct.pack("I", self.pathOffs)) # +20
		f.write(struct.pack("i", npnt)) # +24
		f.write(struct.pack("i", npol)) # +28
		f.write(struct.pack("i", 0)) # +2C -> pnts
		f.write(struct.pack("i", 0)) # +30 -> pols
		f.write(struct.pack("i", 0)) # +34 -> tris
		f.write(struct.pack("i", 0)) # +38 -> idx
		f.write(struct.pack("i", 0)) # +3C -> BVH

		f.write(struct.pack("ffffff", # + 40
			self.bboxMin[0], self.bboxMin[1], self.bboxMin[2],
			self.bboxMax[0], self.bboxMax[1], self.bboxMax[2]))

		f.seek(align(f.tell(), 0x10))
		fpatch(f, 0x30, f.tell()) # -> pols
		for pol in self.pols:
			pol.write(f)

		f.seek(align(f.tell(), 0x10))
		fpatch(f, 0x2C, f.tell()) # -> pnts
		for pnt in self.pnts:
			pos = pnt.position()
			f.write(struct.pack("fff", pos[0], pos[1], pos[2]))

		fpatch(f, 0x38, f.tell()) # -> idx
		for idx in self.idx:
			f.write(struct.pack("i", idx))

		fpatch(f, 0x34, f.tell()) # -> tris
		for pol in self.pols:
			if pol.tris:
				for itri in pol.tris:
					f.write(struct.pack("i", itri))

		if self.bvh:
			#print len(self.pols), "polys ->", len(self.bvh.nodeLst), "BVH nodes"
			fpatch(f, 0x3C, f.tell()) # -> BVH
			self.bvh.write(f)

		fpatch(f, 0x18, f.tell()) # -> strs
		f.write(self.strs.data)

		fpatch(f, 0x14, f.tell()) # +14 := file size

	def save(self, fpath):
		if not self.pols or len(self.pols) < 1: return
		try:
			f = open(fpath, "wb")
		except IOError:
			dbgmsg("Can't create output file: " + fpath)
		else:
			self.write(f)
			f.close()
