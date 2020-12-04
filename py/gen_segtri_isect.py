import os
import sys
import hou
import inspect

def getExecPath():
	return os.path.abspath(inspect.getframeinfo(inspect.currentframe()).filename)

def getExecDir():
	return os.path.dirname(getExecPath())

def saveScalar(f, val):
	f.write("{0:.16g}\t".format(val))

def saveVec(f, v):
	f.write("{0:.16g} {1:.16g} {2:.16g}\t".format(v[0], v[1], v[2]))

def savePrim(f, prim):
	pts = prim.points()
	for pt in pts:
		p = pt.position()
		saveVec(f, p)
		#f.write("\t")
		#f.write("{0:.32g} {1:.32g} {2:.32g}\t".format(p[0], p[1], p[2]))
	f.write("\n")
def saveResult(f, isectPri, hitPos, hitDir):
	saveScalar(f, isectPri)
	saveVec(f, hitPos)
	saveVec(f, hitDir)
	f.write("\n")

def segTriExp(outPath, segNode, triNode, resNode) :
	#segNode = hou.node(segPath)
	segGeo = segNode.geometry()
	segs = segGeo.prims()
	nseg = len(segs)
	print("Num segments : %d" % nseg)

	#resNode = hou.node(resPath)
	resGeo = resNode.geometry()
	resPts = resGeo.points()
	nres = len(resPts)
	print("Num results : %d" % nseg)

	if nseg != nres :
		print("Number of segmentes provided does not much the number of results")
		return

	#triNode = hou.node(triPath)
	triGeo = triNode.geometry()
	tris = triGeo.prims()
	ntri = len(tris)
	print("Num triangles : %d" % ntri)

	f = open(outPath, "w")
	if not f: return

	f.write("{0} {1}\n".format(nseg, ntri))

	for seg in segs:
		savePrim(f, seg)

	for tri in tris:
		savePrim(f, tri)

	for pt in resPts:
		pri = pt.attribValue("isectPri")
		pos = (0,0,0) if pri == -1 else pt.attribValue("isectPos")
		nrm = (0,1,0) if pri == -1 else pt.attribValue("isectN")
		saveResult(f, pri, pos, nrm)

	f.close()


if __name__=="__main__":
	segNode = hou.node('/obj/SEG_TRI_TEST/SEGMENTS')
	triNode = hou.node('/obj/SEG_TRI_TEST/TRIANGLES')
	resNode = hou.node('/obj/SEG_TRI_TEST/RESULTS')
	if segNode and resNode and triNode:
		segTriExp(hou.expandString("$HIP/seg_tri.dat"), segNode, triNode, resNode)