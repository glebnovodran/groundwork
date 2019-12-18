# execfile(hou.expandString("$HIP/_gwexp.py"))

import sys
import hou
import os
import imp
import inspect

libPath = os.path.dirname(os.path.abspath(inspect.getframeinfo(inspect.currentframe()).filename)) + "/../../py"

libName = "gwmdl"
libFile, libFname, libDescr = imp.find_module(libName, [libPath])
imp.load_module(libName, libFile, libFname, libDescr)
from gwmdl import *

libName = "gwcat"
libFile, libFname, libDescr = imp.find_module(libName, [libPath])
imp.load_module(libName, libFile, libFname, libDescr)
from gwcat import *

libName = "copdds"
libFile, libFname, libDescr = imp.find_module(libName, [libPath])
imp.load_module(libName, libFile, libFname, libDescr)
from copdds import *

libName = "tdmot"
libFile, libFname, libDescr = imp.find_module(libName, [libPath])
imp.load_module(libName, libFile, libFname, libDescr)
from tdmot import *

expDir = hou.expandString("$HIP/../../data/cook_rb")
if not os.path.exists(expDir): os.makedirs(expDir)

expMdl = True
expTex = True
expMot = True

cat = GWCatalog()

if expMdl:
	sop = hou.node("/obj/cook_rb/EXP")
	if sop:
		mdlName = "cook_rb.gwmdl"
		dbgmsg("Exporting model: " + sop.path())
		mdr = GWModelResource(sop)
		mdr.save(expDir + "/" + mdlName)
		cat.add(mdlName, GWResKind.MODEL)

if expTex:
	texNet = hou.node("/obj/cook_rb/TEX")
	if texNet:
		texs = []
		for node in texNet.children():
			if node.type().name() == "null": texs.append(node)
		for cop in texs:
			texName = cop.name() + ".dds";
			dbgmsg("Exporting texture: " + cop.path())
			saveDDS(expDir + "/" + texName, cop)
			cat.add(texName, GWResKind.DDS)

if expMot:
	motNet = hou.node("/obj/MOTION")
	if motNet:
		clips = []
		for node in motNet.children():
			if node.type().name() == "null" and not node.isBypassed(): clips.append(node)
		for chop in clips:
			dbgmsg("Exporting motion: " + chop.path())
			motName = chop.name() + ".tdmot"
			saveTDMot(expDir + "/" + motName, chop)
			cat.add(motName, GWResKind.TDMOT)

cat.save(expDir + "/" + "cook_rb.gwcat")
