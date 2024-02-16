#!/usr/bin/python
#
# Inline, Crossline, True dip, Dip Azimuth using the vector filtered envelope weighted phase gradient
# Derivatives calulated using Kroon's 3 point filter
#
import sys,os
import numpy as np
from scipy.signal import hilbert
from numba import jit

#
# Import the module with the I/O scaffolding of the External Attribute
#
sys.path.insert(0, os.path.join(sys.path[0], '..'))
import extattrib as xa
import extlib as xl

#
# These are the attribute parameters
#
xa.params = {
	'Inputs': ['Input'],
	'Output': ['Crl_dip', 'Inl_dip', 'True Dip', 'Dip Azimuth'],
	'ZSampMargin' : {'Value':[-15,15], 'Minimum':[-2,2], 'Symmetric': True},
	'StepOut' : {'Value': [2,2], 'Minimum': [2,2], 'Symmetric': True},
	'Par_0' : {'Name': 'Vector Filter ZStepOut', 'Value': 1},
	'Par_1' : {'Name': 'Band', 'Value': 0.9},
	'Select': {'Name': 'Filter', 'Values': ['Mean Dip', 'Vector L1 Median Dip', 'Vector L2 Median Dip'], 'Selection': 0},
	'Help': xa.HelpRoot+'dipazimuth/#orientation-from-vector-filtered-3d-complex-trace-phase'
}
#
# Define the compute function
#
def doCompute():
	xs = xa.SI['nrinl']
	ys = xa.SI['nrcrl']
	zs = xa.params['ZSampMargin']['Value'][1] - xa.params['ZSampMargin']['Value'][0] + 1
	filt = xa.params['Select']['Selection']
	filtFunc = jit(xl.vecmean) if filt==0 else  jit(xl.vmf_l1) if filt==1 else jit(xl.vmf_l2)
	inlFactor = xa.SI['zstep']/xa.SI['inldist'] * xa.SI['dipFactor']
	crlFactor = xa.SI['zstep']/xa.SI['crldist'] * xa.SI['dipFactor']
	band = xa.params['Par_1']['Value']
	zw = max(2*int(xa.params['Par_0']['Value'])+1,3)
	N = xa.params['ZSampMargin']['Value'][1]
	kernel = xl.hilbert_kernel(N, band)
	while True:
		xa.doInput()

		indata = xa.Input['Input']
#
#	Analytic Signal
#
		ansig = np.apply_along_axis(np.convolve,-1, indata, kernel, mode="same")
		sr = np.real(ansig)
		si = np.imag(ansig)
#
#	Compute partial derivatives
		sx = xl.kroon3( sr, axis=0 )
		sy = xl.kroon3( sr, axis=1 )
		sz = xl.kroon3( sr, axis=2 )
		shx = xl.kroon3( si, axis=0 )
		shy = xl.kroon3( si, axis=1 )
		shz = xl.kroon3( si, axis=2 )

		px = sr[1:xs-1,1:ys-1,:] * shx[1:xs-1,1:ys-1,:] - si[1:xs-1,1:ys-1,:] * sx[1:xs-1,1:ys-1,:]
		py = sr[1:xs-1,1:ys-1,:] * shy[1:xs-1,1:ys-1,:] - si[1:xs-1,1:ys-1,:] * sy[1:xs-1,1:ys-1,:]
		pz = sr[1:xs-1,1:ys-1,:] * shz[1:xs-1,1:ys-1,:] - si[1:xs-1,1:ys-1,:] * sz[1:xs-1,1:ys-1,:]
#
#	Normalise the gradients so Z component is positive
		p = np.sign(pz)/np.sqrt(px*px+py*py+pz*pz)
		px *= p
		py *= p
		pz *= p
#
#	Filter
		out = np.empty((3,xa.TI['nrsamp']))
		xl.vecFilter(np.array([px,py,pz]), zw, filtFunc, out)
#
#	Get the output
		xa.Output['Crl_dip'] = -out[1,:]/out[2,:]*crlFactor
		xa.Output['Inl_dip'] = -out[0,:]/out[2,:]*inlFactor
		xa.Output['True Dip'] = np.sqrt(xa.Output['Crl_dip']*xa.Output['Crl_dip']+xa.Output['Inl_dip']*xa.Output['Inl_dip'])
		xa.Output['Dip Azimuth'] = np.degrees(np.arctan2(xa.Output['Inl_dip'],xa.Output['Crl_dip']))

		xa.doOutput()


#
# Assign the compute function to the attribute
#
xa.doCompute = doCompute
#
# Do it
#
xa.run(sys.argv[1:])

