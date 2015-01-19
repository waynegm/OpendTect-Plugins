#!/usr/bin/python
#
# Calculate inline and crossline dip using the operator proposed by Xudong Jiang in
# "Extracting image orientation feature by using integration operator" 
# Pattern Recognition 40 (2007) 705-717
#
import sys, getopt, os, json
from math import *
import numpy as np
import sympy as sp
from scipy.ndimage import convolve
from scipy.signal import gaussian_filter
#
# These are the attribute parameters
#
params = {
	'Input': 'Input',
	'Output': ['Crl_dip', 'Inl_dip'],
	'ZSampMargin' : [-10,10],
	'StepOut' : [1,1],
}

def doCompute():
	global Output
	global inl_kernel
	global crl_kernel
	
	inl_kernel = xukernel(SI['nrcrl'].item())
	crl_kernel = xukernel(SI['nrinl'].item())
#
# index of current trace position in Input numpy array
#
	ilndx = SI['nrinl']//2
	crldx = SI['nrcrl']//2
	while True:
		doInput()
		Output['Crl_dip'] = xudip(inl_kernel, Input[ilndx,:,:])
		Output['Inl_dip'] = xudip(crl_kernel, Input[:,crldx,:])
		doOutput()
	

#
# Apply the orientation operator kernel
#
def xudip(kernel, inarr):
    sz = np.shape(kernel)[0]
    M2 = sz//2
    r = convolve(inarr, kernel.T)
    i = convolve(inarr, kernel)
    res = np.where(r[M2,:] != 0.0, -i[M2,:]/r[M2,:], 0.0)
    
    return res

#
# Compute the orientation operator kernel
#
def xukernel(size):
    s = sp.Symbol('s',integer=True)
    b = sp.Symbol('b', integer=True)
    _M = sp.Symbol('M',integer=True)
    _mp = sp.Symbol('mp', integer=True)
    p = sp.Symbol('p')
    p = (sp.Product(s-b, (b,0,_M))/(s-_mp)).doit()
    res = np.zeros((size, size))
    M = size-1
    M2 = M//2
    for n in range(1,M2+1):
        nmm = M2 - n
        npp = M2 + n
        for m in range(0,M2+1):
            mmm = M2 - m
            mpp = M2 + m
            const = (-1.0)**mmm/(factorial(mmm)*factorial(mpp))
            v = p.subs(_M,M).subs(_mp,mpp)
            res[npp,mpp] = const*sp.re(sp.N(sp.integrate(v,(s,nmm,npp))))
            res[npp,mmm] = res[npp,mpp]
            res[nmm,mpp] = -res[npp,mpp]
            res[nmm,mmm] = res[nmm,mpp]
    
    return res

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
  
def readPar(jsonStr):
	try:
		global params
		params.update(json.loads(jsonStr))
	except (TypeError, ValueError) as err:
		print('Error decoding parameter string: %s' % err, file=sys.stderr)

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
  

