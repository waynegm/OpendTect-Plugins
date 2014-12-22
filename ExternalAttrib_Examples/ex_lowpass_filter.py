#!/usr/bin/python
#
# Simple Butterworth LowPass Filter using Numpy/Scipy
# for the OpendTect ExternalAttrib plugin
#
import sys, getopt, os, json
import numpy as np
import scipy.signal as sig
#
# These are the attribute parameters
#
params = {
	'Input': 'Filter Input',
	'ZSampMargin' : [-30,30],
	'Par_0' : {'Name': 'Filter Cutoff', 'Value': 40},
	'Par_1' : {'Name': 'Filter Order', 'Value': 3}
}

def doCompute():
	global Output
	order = params['Par_1']['Value']
	nyquist = 1.0/(2.0*SI['zstep'])
	cutoff = params['Par_0']['Value']/nyquist
	b, a = sig.butter(order, cutoff, 'low', analog=False)
	while True:
		doInput()
		Output = sig.filtfilt(b, a, Input[0,0,:], padtype=None, padlen=0)
		doOutput()
	
#------------------------------------------------------------------------------
# Should not need to change anything below this point
#------------------------------------------------------------------------------
def doInput():
	global Input
	global TI
	TI = np.frombuffer(sys.stdin.read(dt_trcInfo.itemsize), dtype=dt_trcInfo, count=1)[0]
	nrsamples = TI['nrsamp']*SI['nrtraces']
	Input = np.reshape(np.frombuffer(sys.stdin.read(nrsamples*4), dtype="f4", count=nrsamples),(SI['nrinl'],SI['nrcrl'],TI['nrsamp']))

def doOutput():
	global Output
	sys.stdout.write(Output.astype(np.float32,copy=False).tobytes())
	sys.stdout.flush()
	
	
def writePar():
	try:
		json.dump(params, sys.stdout)
		sys.stdout.flush()
	except (TypeError, ValueError) as err:
		print('Error exporting parameter string: %s'% err, file=sys.stderr)
  
def readPar(jsonStr):
	try:
		global params
		params = json.loads(jsonStr)
	except (TypeError, ValueError) as err:
		print('Error decoding parameter string: %s' % err, file=sys.stderr)

def preCompute():
	global dt_trcInfo
	global SI
	sys.stdin = os.fdopen(sys.stdin.fileno(), 'rb', 0) 
	sys.stdout = os.fdopen(sys.stdout.fileno(), 'wb', 0)
	dt_trcInfo = np.dtype([	('nrsamp','i4'),
							('z0','i4'),
							('inl','i4'),
							('crl','i4')])
	dt_seisInfo = np.dtype([('nrtraces','i4'),
							('nrinl','i4'),
							('nrcrl','i4'),
							('zstep','f4'),
							('inldist','f4'),
							('crldist','f4')])
	SI = np.frombuffer(sys.stdin.read(dt_seisInfo.itemsize), dtype=dt_seisInfo, count=1)[0]

def usage():
	print("Usage: %s \n" % sys.argv[0])

def main(argv):
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

if __name__ == "__main__":
  main(sys.argv[1:])
  

