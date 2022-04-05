#!/usr/bin/python
#
# Inline, Crossline, True dip or Dip Azimuth using the vector filtered gradient
# Derivatives calulated using Kroon's 3 point filter
#
import sys,os
import numpy as np
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
	'ZSampMargin' : {'Value':[-2,2], 'Minimum':[-2,2], 'Symmetric': True},
	'StepOut' : {'Value': [2,2], 'Minimum': [2,2]},
	'Select': {'Name': 'Filter', 'Values': ['Mean Dip', 'Vector L1 Median Dip', 'Vector L2 Median Dip', 'Vector X3 Median'], 'Selection': 0},
	'Help': 'http://waynegm.github.io/OpendTect-Plugin-Docs/external_attributes/DipandAzimuth.html#orientation-from-vector-filtered-gradients'
}
#
# Define the compute function
#
def doCompute():
	xs = xa.SI['nrinl']
	ys = xa.SI['nrcrl']
	zs = xa.params['ZSampMargin']['Value'][1] - xa.params['ZSampMargin']['Value'][0] + 1
	zw = max(zs-2, 3)
	filt = xa.params['Select']['Selection']
	filtFunc = jit(xl.vecmean) if filt==0 else  jit(xl.vmf_l1) if filt==1 else jit(xl.vmf_l2) if filt==2 else jit(xl.vmf_x3)
	inlFactor = xa.SI['zstep']/xa.SI['inldist'] * xa.SI['dipFactor']
	crlFactor = xa.SI['zstep']/xa.SI['crldist'] * xa.SI['dipFactor']
	while True:
		xa.doInput()

		p = xa.Input['Input']
#
#	Compute partial derivatives
		px = xl.kroon3( p, axis=0 )[1:xs-1,1:ys-1,:]
		py = xl.kroon3( p, axis=1 )[1:xs-1,1:ys-1,:]
		pz = xl.kroon3( p, axis=2 )[1:xs-1,1:ys-1,:]
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

