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

libName = "copdds"
libFile, libFname, libDescr = imp.find_module(libName, [libPath])
imp.load_module(libName, libFile, libFname, libDescr)
from copdds import *

libName = "tdmot"
libFile, libFname, libDescr = imp.find_module(libName, [libPath])
imp.load_module(libName, libFile, libFname, libDescr)
from tdmot import *

expDir = hou.expandString("$HIP/EXP")
if not os.path.exists(expDir): os.makedirs(expDir)

expMdl = True
expTex = True
expMot = True

if expMdl:
	sop = hou.node("/obj/cook_rb/EXP")
	if sop:
		dbgmsg("Exporting model: " + sop.path())
		mdr = GWModelResource(sop)
		mdr.save(expDir + "/cook_rb.gwmdl")

if expTex:
	texNet = hou.node("/obj/cook_rb/TEX")
	if texNet:
		texs = []
		for node in texNet.children():
			if node.type().name() == "null": texs.append(node)
		for cop in texs:
			dbgmsg("Exporting texture: " + cop.path())
			saveDDS(expDir + "/" + cop.name() + ".dds", cop)

if expMot:
	motNet = hou.node("/obj/MOTION")
	if motNet:
		clips = []
		for node in motNet.children():
			if node.type().name() == "null" and not node.isBypassed(): clips.append(node)
		for chop in clips:
			dbgmsg("Exporting motion: " + chop.path())
			saveTDMot(expDir + "/" + chop.name() + ".txt", chop)
