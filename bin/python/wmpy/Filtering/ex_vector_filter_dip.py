#!/usr/bin/python
#
# Vector Filter given Inline and Crossline dip
#
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
	'Inputs': ['Crl_dip', 'Inl_dip'],
	'Output': ['Crl_dip', 'Inl_dip', 'True Dip', 'Dip Azimuth'],
	'ZSampMargin' : {'Value':[-3,3], 'Symmetric': True},
	'StepOut' : {'Value': [3,3], 'Symmetric': True},
	'Select': {'Name': 'Filter', 'Values': ['Mean Dip', 'Vector L1 Median Dip', 'Vector L2 Median Dip'], 'Selection': 0},
	'Help': 'http://waynegm.github.io/WMPlugin-Docs/docs/externalattributes/vectorfilter/'
}
#
# Define the compute function
#
def doCompute():
	inlFactor = xa.SI['zstep']/xa.SI['inldist'] * xa.SI['dipFactor']
	crlFactor = xa.SI['zstep']/xa.SI['crldist'] * xa.SI['dipFactor']
	zw = xa.params['ZSampMargin']['Value'][1] - xa.params['ZSampMargin']['Value'][0] + 1
	zw = max(zw, 3)
	filt = xa.params['Select']['Selection']
	filtFunc = jit(xl.vecmean) if filt==0 else  jit(xl.vmf_l1) if filt==1 else jit(xl.vmf_l2)
	while True:
		xa.doInput()

		dx = -xa.Input['Inl_dip']/inlFactor
		dy = -xa.Input['Crl_dip']/crlFactor
		dz = np.ones(dx.shape)
		s = np.sqrt(dx*dx+dy*dy+dz*dz)
#
#	Apply the Vector Filter
		out = np.empty((3,xa.TI['nrsamp']))
		xl.vecFilter(np.array([dx/s,dy/s,dz/s]), zw, filtFunc, out)
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

