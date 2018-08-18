import os
import sys
import hou

def saveTDMot(path, chop, rows = True):
	f = open(path, "w")
	if not f: return
	if rows:
		for i, trk in enumerate(chop.tracks()):
			f.write(trk.name())
			for val in trk.allSamples():
				f.write("\t{0:.08f}".format(val))
			f.write('\n')
	else:
		for i, trk in enumerate(chop.tracks()):
			if i > 0: f.write('\t')
			f.write(trk.name())
		f.write('\n')
		smps = []
		nfrm = 0
		for trk in chop.tracks():
			data = trk.allSamples()
			nfrm = max(nfrm, len(data))
			smps.append(data)
		ntrk = len(chop.tracks())
		for ifrm in range(nfrm):
			for itrk in range(ntrk):
				if itrk > 0: f.write('\t')
				f.write("{0:.8f}".format(smps[itrk][ifrm]))
			f.write('\n')
	f.close()
