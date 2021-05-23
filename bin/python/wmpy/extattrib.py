#
# Python External Attribute Support
#
# Copyright (C) 2016 Wayne Mogg All rights reserved.
#
# This file may be used under the terms of the MIT License
# (https://github.com/waynegm/OpendTect-External-Attributes/blob/master/LICENSE)
#
# Author:		Wayne Mogg
# Date: 		March, 2016
# Homepage:		http://waynegm.github.io/OpendTect-Plugin-Docs/external_attributes/
#
import sys, getopt, os, json, urllib
import numpy as np

import logging

logH = logging.getLogger(os.path.basename(sys.argv[0])+" - "+str(os.getpid()))
logFormatter = logging.Formatter('%(levelname)s - %(name)s - %(message)s')
lH = logging.StreamHandler()
lH.setFormatter(logFormatter)
logH.addHandler(lH)

params = {}
Input = {}
Output = {}
TI = {}
SI = {}
undef = 1e30

def doCompute():
    global Output
    while true:
        doInput()
        Output = Input
        doOutput()

def doInput():
	global Input
	global TI
	TI = np.frombuffer(sys.stdin.read(dt_trcInfo.itemsize), dtype=dt_trcInfo, count=1)[0]
	nrsamples = TI['nrsamp']*SI['nrtraces']
	if 'Inputs' in params:
		for inp in params['Inputs']:
			Input[inp] = np.reshape(np.frombuffer(sys.stdin.read(nrsamples*4), dtype="f4", count=nrsamples),(SI['nrinl'],SI['nrcrl'],TI['nrsamp']))
	else:
		Input = np.reshape(np.frombuffer(sys.stdin.read(nrsamples*4), dtype="f4", count=nrsamples),(SI['nrinl'],SI['nrcrl'],TI['nrsamp']))

def doOutput():
	global Output
	if 'Output' in params:
	  for out in params['Output']:
	    Output[out][np.isnan(Output[out])] = undef
	    sys.stdout.write(Output[out].astype(np.float32,copy=False).tobytes())
	else:
	    Output[np.isnan(Output)] = undef
	    sys.stdout.write(Output.astype(np.float32,copy=False).tobytes())
	sys.stdout.flush()
	
def writePar():
	try:
		print(urllib.parse.quote(json.dumps(params)), file=sys.stdout)
		sys.stdout.flush()
	except (TypeError, ValueError) as err:
		logH.error('Error exporting parameter string: %s'% err)
		sys.exit(1)
  
def readPar(jsonStr):
	global params
	try:
		params.update(json.loads(urllib.parse.unquote(jsonStr)))
	except (TypeError, ValueError) as err:
		logH.error('Error decoding parameter string: %s' % err)
		sys.exit(1)

def preCompute():
	global dt_trcInfo
	global SI
	global Output
	Output = {}
	sys.stdin = os.fdopen(sys.stdin.fileno(), 'rb', 0) 
	sys.stdout = os.fdopen(sys.stdout.fileno(), 'wb', 0)
	dt_trcInfo = np.dtype([	('nrsamp','i4'),
							('z0','i4'),
							('inl','i4'),
							('crl','i4')])
	dt_seisInfo = np.dtype([('nrtraces','i4'),
							('nrinput','i4'),
							('nroutput','i4'),
							('nrinl','i4'),
							('nrcrl','i4'),
							('zstep','f4'),
							('inldist','f4'),
							('crldist','f4'),
							('zFactor','f4'),
							('dipFactor','f4')])
	SI = np.frombuffer(sys.stdin.read(dt_seisInfo.itemsize), dtype=dt_seisInfo, count=1)[0]

def usage():
	print("Usage: %s \n" % sys.argv[0])

def run(argv):
	global logH
	try:
		opts, args = getopt.getopt(argv,"hgc:",["help", "getpar", "compute="])
	except getopt.GetoptError as e:
		logH.error('Error in command line parameters: %s' % e)
		sys.exit(2)
	for opt, arg in opts:
		if opt in ("-h", "--help"):
			usage()
			sys.exit()
		elif opt in ("-g", "--getpar"):
			try:
				writePar()
				sys.exit()
			except Exception:
				logH.error("Fatal error in getpar", exc_info=True)
		elif opt in ("-c", "--compute"):
			try:
				readPar(arg)
				preCompute()
				doCompute()
				sys.exit()
			except Exception:
				logH.error("Fatal error in compute", exc_info=True)

  

