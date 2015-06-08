#
# Python External Attribute Support
#
import sys, getopt, os, json
import numpy as np

params = {}
Input = {}
Output = {}
TI = {}
SI = {}

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
	    sys.stdout.write(Output[out].astype(np.float32,copy=False).tobytes())
	else:
	    sys.stdout.write(Output.astype(np.float32,copy=False).tobytes())
	sys.stdout.flush()
	
def writePar():
	try:
		json.dump(params, sys.stdout)
		sys.stdout.flush()
	except (TypeError, ValueError) as err:
		print('Error exporting parameter string: %s'% err, file=sys.stderr)
		sys.exit(1)
  
def readPar(jsonStr):
	global params
	try:
		params.update(json.loads(jsonStr))
	except (TypeError, ValueError) as err:
		print('Error decoding parameter string: %s' % err, file=sys.stderr)
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
	try:
		opts, args = getopt.getopt(argv,"hgc:",["help", "getpar", "compute="])
	except getopt.GetoptError as e:
		print(string(e))
		usage()
		sys.exit(2)
	for opt, arg in opts:
		if opt in ("-h", "--help"):
			usage()
			sys.exit()
		elif opt in ("-g", "--getpar"):
			writePar()
			sys.exit()
		elif opt in ("-c", "--compute"):
			readPar(arg)
			preCompute()
			doCompute()
			sys.exit()

  

