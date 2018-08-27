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

def f32bits(x):
	return struct.unpack("I", struct.pack("f", x))[0]

def floatToHalf(x):
	if x == 0: return 0
	bits = f32bits(x)
	s = (bits >> 16) & (1 << 15)
	bits &= 0x7FFFFFFF
	if bits > 0x477FE000: return 0x7C00 | s
	e = ((bits & 0x7F800000) >> 23) - 127 + 15
	if e < 0: return 0
	if e > 31: e = 31
	m = bits & 0x7FFFFF
	return s | ((e << 10) & 0x7C00) | ((m >> 13) & 0x3FF)

def encodeOcta(v):
	x = v[0]
	y = v[1]
	z = v[2]
	ax = abs(x)
	ay = abs(y)
	az = abs(z)
	d = ax + ay + az
	if d != 0: d = 1.0 / d
	ox = x * d
	oy = y * d
	if z < 0.0:
		tx = (1.0 - abs(oy))
		if ox < 0: tx = -tx
		ty = (1.0 - abs(ox))
		if oy < 0: ty = -ty
		ox = tx
		oy = ty
	return [ox, oy]

def fresnelFromIOR(ior):
	r = (ior - 1.0) / (ior + 1.0)
	return r * r

def encodePntAttr(nrm, tng, clr, tex):
	onrm = encodeOcta(nrm)
	otng = encodeOcta(tng)
	hnto = [floatToHalf(onrm[0]), floatToHalf(onrm[1]), floatToHalf(otng[0]), floatToHalf(otng[1])]
	hclr = [floatToHalf(clr[0]), floatToHalf(clr[1]), floatToHalf(clr[2]), floatToHalf(clr[3])]
	htex = [floatToHalf(tex[0]), floatToHalf(tex[1]), floatToHalf(tex[2]), floatToHalf(tex[3])]
	return struct.pack("HHHHHHHHHHHH",
	                    hnto[0], hnto[1], hnto[2], hnto[3],
	                    hclr[0], hclr[1], hclr[2], hclr[3],
	                    htex[0], htex[1], htex[2], htex[3])

def encodeMtx(mtx):
	t = mtx.asTuple()
	s = ""
	for x in t: s += struct.pack("f", x)
	return s

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

class Tri:
	def __init__(self, mdl, prim):
		self.mdl = mdl
		self.prim = prim
		self.mtlId = 0
		self.pts = []
		for vtx in prim.vertices():
			self.pts.append(vtx.point().number())

	def minIdx(self):
		return min(min(self.pts[0], self.pts[1]), self.pts[2])

	def maxIdx(self):
		return max(max(self.pts[0], self.pts[1]), self.pts[2])

class Material:
	def __init__(self, mdl, path):
		self.mdl = mdl
		self.path = path
		self.pathOffs = -1
		self.setDefaultParams()
		if path:
			self.node = hou.node(path)
			self.tris = []
			if self.node:
				mtlType = self.node.type().name()
				if mtlType == "classicshader":
					if self.node.parm("diff_colorUseTexture").evalAsInt():
						self.baseMapPath = self.node.parm("diff_colorTexture").evalAsString()
					self.baseColor = self.node.parmTuple("diff_color").eval()
					self.specColor = self.node.parmTuple("spec_color").eval()
					self.roughness = self.node.parm("spec_rough").evalAsFloat()
					self.fresnel = fresnelFromIOR(self.node.parm("ior_in").evalAsFloat())
					self.bumpScale = self.node.parm("baseNormal_scale").evalAsFloat()
					self.setSemiTransparent(self.node.parm("diff_colorTextureUseAlpha").evalAsInt())
					self.setFlipTangent(self.node.parm("baseNormal_flipX").evalAsInt())
					self.setFlipBitangent(self.node.parm("baseNormal_flipY").evalAsInt())
				elif mtlType == "principledshader":
					if self.node.parm("basecolor_useTexture").evalAsInt():
						self.baseMapPath = self.node.parm("basecolor_texture").evalAsString()
					self.baseColor = self.node.parmTuple("basecolor").eval()
					self.roughness = self.node.parm("rough").evalAsFloat()
				if self.baseMapPath:
					self.baseMapPathOffs = self.mdl.strs.add(self.baseMapPath)
		else:
			self.node = None
			self.tris = None
		self.idxOrg = -1

	def getNumTris(self):
		if self.tris: return len(self.tris)
		return len(self.mdl.tris)

	def setDefaultParams(self):
		self.baseColor = [1.0, 1.0, 1.0]
		self.specColor = [1.0, 1.0, 1.0]
		self.roughness = 0.75
		self.fresnel = fresnelFromIOR(1.33)
		self.bumpScale = 0.0
		self.flags = 0
		self.baseMapPath = None
		self.baseMapPathOffs = -1
		self.extParams = None
		self.extParamsOffs = -1

	def addExtParams(self, paramsStr):
		self.extParams = paramsStr
		self.extParamsOffs = self.mdl.strs.add(self.extParams)

	def setFlag(self, idx, val):
		mask = 1 << idx
		if val:
			self.flags |= mask
		else:
			self.flags &= ~mask

	def setDoubleSided(self, flg): self.setFlag(0, flg)
	def setSemiTransparent(self, flg): self.setFlag(1, flg)
	def setFlipTangent(self, flg): self.setFlag(2, flg)
	def setFlipBitangent(self, flg): self.setFlag(3, flg)

	def calcIdxRange(self):
		if self.tris:
			for i, triIdx in enumerate(self.tris):
				tri = self.mdl.tris[triIdx]
				if i == 0:
					self.minIdx = tri.minIdx()
					self.maxIdx = tri.maxIdx()
				else:
					self.minIdx = min(self.minIdx, tri.minIdx())
					self.maxIdx = max(self.maxIdx, tri.maxIdx())
		else:
			self.minIdx = 0
			self.maxIdx = len(self.mdl.pts) - 1

	def getPath(self):
		if self.path:
			return self.path
		return "$sys/default"

	def isShortIdx(self):
		return (self.maxIdx - self.minIdx) < (1 << 16)

	def write(self, f):
		f.write(struct.pack("i", self.pathOffs)) # 00
		f.write(struct.pack("I", self.flags)) # 04
		f.write(struct.pack("i", self.baseMapPathOffs)) # 08
		f.write(struct.pack("i", self.extParamsOffs)) # 0C
		f.write(struct.pack("i", self.idxOrg)) # 10
		f.write(struct.pack("i", self.getNumTris())) # 14
		f.write(struct.pack("i", self.minIdx)) # 18
		f.write(struct.pack("i", self.maxIdx)) # 1C
		f.write(struct.pack("fff", self.baseColor[0], self.baseColor[1], self.baseColor[2])) # 20
		f.write(struct.pack("fff", self.specColor[0], self.specColor[1], self.specColor[2])) # 2C
		f.write(struct.pack("f", self.roughness)) # 38
		f.write(struct.pack("f", self.fresnel)) # 3C
		f.write(struct.pack("f", self.bumpScale)) # 40

class SkelNode:
	def __init__(self, mdl, hnode):
		self.mdl = mdl
		self.hnode = hnode
		self.nameOffs = mdl.strs.add(self.hnode.name())
		self.wmtx = self.hnode.worldTransform()
		self.lmtx = self.hnode.localTransform()
		inp = self.hnode.inputConnectors()[0]
		if len(inp):
			self.parent = inp[0].inputNode()
		else:
			self.parent = None

class GWModelResource:
	def __init__(self, sop, extInfo = None, skelRootPath = "/obj/REST/root"):
		self.sop = sop
		self.geo = sop.geometry()
		self.pts = self.geo.points()
		self.strs = Strings()
		self.pathOffs = self.strs.add(self.sop.parent().path())
		self.extInfo = extInfo
		self.extInfoOffs = -1
		if self.extInfo: self.extInfoOffs = self.strs.add(self.extInfo)
		self.tris = []
		for prim in self.geo.prims():
			if prim.type() == hou.primType.Polygon and len(prim.vertices()) == 3:
				self.tris.append(Tri(self, prim))
		if len(self.tris) < 1:
			dbgmsg("Triangulate geometry first!")
			return
		self.mtls = []
		mtlAttr = self.geo.findPrimAttrib("shop_materialpath")
		if mtlAttr:
			self.mtlMap = {}
			for tri in self.tris:
				mtlPath = tri.prim.attribValue(mtlAttr)
				if not mtlPath in self.mtlMap:
					mtlId = len(self.mtls)
					self.mtlMap[mtlPath] = mtlId
					tri.mtlId = mtlId
					self.mtls.append(Material(self, mtlPath))
				else:
					tri.mtlId = self.mtlMap[mtlPath]
			for i, tri in enumerate(self.tris):
				self.mtls[tri.mtlId].tris.append(i)
		else:
			self.mtls.append(Material(self, None))
		self.nidx16 = 0
		self.nidx32 = 0
		for mtl in self.mtls:
			mtl.pathOffs = self.strs.add(mtl.getPath())
			mtl.calcIdxRange()
			nidx = mtl.getNumTris() * 3
			if mtl.isShortIdx():
				mtl.idxOrg = self.nidx16
				self.nidx16 += nidx
			else:
				mtl.idxOrg = self.nidx32
				self.nidx32 += nidx

		self.skinNodeNames = None
		skinAttr = self.geo.findPointAttrib("boneCapture")
		if skinAttr:
			self.skinNodeNames = []
			self.skinNodeNameToOffs = {}
			self.skinNodeNameToId = {}
			tbl = skinAttr.indexPairPropertyTables()[0]
			n = tbl.numIndices()
			for i in xrange(n):
				name = tbl.stringPropertyValueAtIndex("pCaptPath", i)
				name = name.split("/cregion")[0]
				nameOffs = self.strs.add(name)
				self.skinNodeNameToOffs[name] = nameOffs
				self.skinNodeNameToId[name] = len(self.skinNodeNames)
				self.skinNodeNames.append(name)
			self.skinData = []
			for ipnt, pnt in enumerate(self.pts):
				skin = pnt.floatListAttribValue(skinAttr)
				nwgt = len(skin) / 2
				iw = []
				for i in xrange(nwgt):
					idx = int(skin[i*2])
					if idx >= 0: iw.append([idx, skin[i*2 + 1]])
				iw.sort(key = lambda iw: -abs(iw[1]))
				if len(iw) > 4:
					dbgmsg("Warning: too many weights at point {}".format(ipnt))
					iw = iw[:4]
				self.skinData.append(iw)
		else:
			skelRootPath = None # no skin -> no skel

		self.skelNodes = None
		if skelRootPath:
			skelRoot = hou.node(skelRootPath)
			if skelRoot:
				self.skelNodes = []
				self.skelNodeMap = {}
				self.skelTree(skelRoot)
				for skelNode in self.skelNodes:
					if skelNode.parent:
						skelNode.parentId = self.skelNodeMap[skelNode.parent.name()]
					else:
						skelNode.parentId = -1

	def skelTree(self, node):
		self.skelNodeMap[node.name()] = len(self.skelNodes)
		self.skelNodes.append(SkelNode(self, node))
		for link in node.outputConnectors()[0]:
			self.skelTree(link.outputNode())

	def getSkinNum(self):
		if self.skinNodeNames: return len(self.skinNodeNames)
		return 0

	def getSkelNum(self):
		if self.skelNodes: return len(self.skelNodes)
		return 0

	def write(self, f):
		sig = "GWModel"
		ver = 0x100
		writeRsrcSig(f, sig, ver)
		f.write(struct.pack("i", 0)) # +14: file size
		f.write(struct.pack("I", 0)) # +18 -> strs
		f.write(struct.pack("I", len(self.strs.data))) # +1C
		npnt = len(self.pts)
		ntri = len(self.tris)
		nmtl = len(self.mtls)
		nskn = self.getSkinNum()
		nskl = self.getSkelNum()
		f.write(struct.pack("I", self.pathOffs)) # +20
		f.write(struct.pack("i", npnt)) # +24
		f.write(struct.pack("i", ntri)) # +28
		f.write(struct.pack("i", nmtl)) # +2C
		f.write(struct.pack("i", self.nidx16)) # +30
		f.write(struct.pack("i", self.nidx32)) # +34
		f.write(struct.pack("i", nskn)) # +38 nskin
		f.write(struct.pack("i", nskl)) # +3C nskel
		f.write(struct.pack("I", 0)) # +40 -> pts
		f.write(struct.pack("I", 0)) # +44 -> attrs
		f.write(struct.pack("I", 0)) # +48 -> mtls
		f.write(struct.pack("I", 0)) # +4C -> idx16
		f.write(struct.pack("I", 0)) # +50 -> idx32
		f.write(struct.pack("I", 0)) # +54 -> skin
		f.write(struct.pack("I", 0)) # +58 -> skel
		f.write(struct.pack("I", 0)) # +5C attr mask
		f.write(struct.pack("I", 0)) # +60 ext info offs

		f.seek(align(f.tell(), 0x10))
		fpatch(f, 0x40, f.tell()) # -> pts
		for pnt in self.pts:
			pos = pnt.position()
			f.write(struct.pack("fff", pos[0], pos[1], pos[2]))

		nmAttr = self.geo.findPointAttrib("N")
		cdAttr = self.geo.findPointAttrib("Cd")
		uvAttr = self.geo.findPointAttrib("uv")
		aoAttr = self.geo.findPointAttrib("AO")
		tuAttr = self.geo.findPointAttrib("tangentu")
		uv2Attr = self.geo.findPointAttrib("uv2")
		alfAttr = self.geo.findPointAttrib("Alpha")
		attrMask = 0
		if nmAttr: attrMask |= 1
		if tuAttr: attrMask |= 1 << 1
		if cdAttr: attrMask |= 1 << 2
		if uvAttr: attrMask |= 1 << 3
		if uv2Attr: attrMask |= 1 << 4
		if alfAttr: attrMask |= 1 << 5
		if aoAttr: attrMask |= 1 << 6
		fpatch(f, 0x5C, attrMask)
		f.seek(align(f.tell(), 0x10))
		fpatch(f, 0x44, f.tell()) # -> attrs
		for pnt in self.pts:
			if nmAttr:
				nrm = pnt.attribValue(nmAttr)
			else:
				nrm = [0.0, 1.0, 0.0]

			if tuAttr:
				tng = pnt.attribValue(tuAttr)
			else:
				tng = [1.0, 0.0, 0.0]

			if cdAttr:
				rgb = pnt.attribValue(cdAttr)
			else:
				rgb = [1.0, 1.0, 1.0]

			if alfAttr:
				a = pnt.attribValue(alfAttr)
			elif aoAttr:
				a = pnt.attribValue(aoAttr)
			else:
				a = 1.0

			if uvAttr:
				uv = pnt.attribValue(uvAttr)
			else:
				uv = [0.0, 0.0]

			if uv2Attr:
				uv2 = pnt.attribValue(uv2Attr)
			else:
				uv2 = uv

			attr = encodePntAttr(nrm, tng,
			                     [rgb[0], rgb[1], rgb[2], a],
			                     [uv[0], 1.0 - uv[1], uv2[0], 1.0 - uv2[1]])
			f.write(attr)

		f.seek(align(f.tell(), 0x10))

		fpatch(f, 0x48, f.tell()) # -> mtls
		for mtl in self.mtls:
			mtl.write(f)

		if self.nidx16 > 0:
			fpatch(f, 0x4C, f.tell()) # -> idx16
			for mtl in self.mtls:
				if mtl.tris: tris = mtl.tris
				else: tris = xrange(len(self.tris))
				for triIdx in tris:
					for idx in self.tris[triIdx].pts:
						rel = (idx - mtl.minIdx) & 0xFFFF
						f.write(struct.pack("H", rel))
		if self.nidx32 > 0:
			f.seek(align(f.tell(), 0x10))
			fpatch(f, 0x50, f.tell()) # -> idx32
			for mtl in self.mtls:
				if mtl.tris: tris = mtl.tris
				else: tris = xrange(len(self.tris))
				for triIdx in tris:
					for idx in self.tris[triIdx].pts:
						rel = (idx - mtl.minIdx)
						f.write(struct.pack("I", rel))

		if nskn > 0:
			f.seek(align(f.tell(), 0x10))
			fpatch(f, 0x54, f.tell()) # -> skin
			for skinName in self.skinNodeNames:
				offs = self.skinNodeNameToOffs[skinName]
				f.write(struct.pack("I", offs))
			# skin -> skel mapping
			for skinName in self.skinNodeNames:
				skelIdx = -1
				if self.skelNodes:
					skelIdx = self.skelNodeMap[skinName]
				f.write(struct.pack("i", skelIdx))
			if self.skinData:
				for iw in self.skinData:
					nwgt = len(iw)
					jidx = [0 for i in xrange(4)]
					jwgt = [0 for i in xrange(4)]
					wsum = 0
					for i in xrange(nwgt):
						jidx[i] = iw[i][0]
						jwgt[i] = int(round(iw[i][1] * 255.0))
						wsum += jwgt[i]
					if wsum != 0xFF:
						jwgt[nwgt-1] = 0xFF - sum(jwgt[:nwgt-1])
					jfmt = "HHHH"
					if len(self.skinNodeNames) <= (1 << 8): jfmt = "BBBB"
					f.write(struct.pack(jfmt, jidx[0], jidx[1], jidx[2], jidx[3]))
					f.write(struct.pack("BBBB", jwgt[0], jwgt[1], jwgt[2], jwgt[3]))

		if nskl > 0:
			f.seek(align(f.tell(), 0x10))
			fpatch(f, 0x58, f.tell()) # -> skel
			mdata = ""
			for skelNode in self.skelNodes:
				mdata += encodeMtx(skelNode.lmtx)
			f.write(mdata)
			for skelNode in self.skelNodes:
				f.write(struct.pack("i", skelNode.nameOffs))
			for skelNode in self.skelNodes:
				f.write(struct.pack("i", skelNode.parentId))

		fpatch(f, 0x18, f.tell()) # -> strs
		f.write(self.strs.data)

		fpatch(f, 0x14, f.tell()) # +14 := file size

	def save(self, fpath):
		if len(self.tris) < 1: return
		try:
			f = open(fpath, "wb")
		except IOError:
			dbgmsg("Can't create output file: " + fpath)
		else:
			self.write(f)
			f.close()

if __name__=="__main__":
	sop = hou.node("/obj/cook_rb/EXP")
	if sop:
		mdd = GWModelResource(sop)
		mdd.save(getExecDir() + "/_test.gwmdl")
