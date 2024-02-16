#!/usr/bin/python
#
# Eigenvalues of the Gradient Structure Tensor
#
# Calculates the eigenvalues of the gradient structure tensor.
# 
# The eigenvalues are numbered in decreasing order of their magnitude.
#
import sys,os
import numpy as np
#
# Import the module with the I/O scaffolding of the External Attribute
#
sys.path.insert(0, os.path.join(sys.path[0], '..', '..'))
import extattrib as xa
import extlib as xl
#
# These are the attribute parameters
#
xa.params = {
	'Inputs': ['In-line gradient', 'Cross-line gradient', 'Z gradient'],
	'Output': ['e1', 'e2', 'e3'],
	'ZSampMargin' : {'Value':[-3,3], 'Symmetric': True},
	'StepOut' : {'Value': [3,3], 'Symmetric': True},
	'Help': xa.HelpRoot
}
#
# Define the compute function
#
def doCompute():
	xs = xa.SI['nrinl']
	ys = xa.SI['nrcrl']
	zs = xa.params['ZSampMargin']['Value'][1] - xa.params['ZSampMargin']['Value'][0] + 1
	kernel = xl.getGaussian(xs, ys, zs)
	while True:
		xa.doInput()

		gx = xa.Input['In-line gradient']
		gy = xa.Input['Cross-line gradient']
		gz = xa.Input['Z gradient']
#
#	Inner product of  gradients
		gx2 = gx * gx
		gy2 = gy * gy
		gz2 = gz * gz
		gxgy = gx * gy
		gxgz = gx * gz
		gygz = gy * gz
#
#	Outer gaussian smoothing
		rgx2 = xl.sconvolve(gx2, kernel)
		rgy2 = xl.sconvolve(gy2, kernel)
		rgz2 = xl.sconvolve(gz2, kernel)
		rgxgy = xl.sconvolve(gxgy, kernel)
		rgxgz = xl.sconvolve(gxgz, kernel)
		rgygz = xl.sconvolve(gygz, kernel)
#
#	Form the structure tensor
		T = np.rollaxis(np.array([	[rgx2,  rgxgy, rgxgz],
									[rgxgy, rgy2,  rgygz],
									[rgxgz, rgygz, rgz2 ]]), 2)
#
#	Get the eigenvalues
		w = np.linalg.eigvalsh(T)
		v = np.rollaxis(np.sort(w),1)
		xa.Output['e1'] = v[2,:]
		xa.Output['e2'] = v[1,:]
		xa.Output['e3'] = v[0,:]
		xa.doOutput()
	
#
# Assign the compute function to the attribute
#
xa.doCompute = doCompute
#
# Do it
#
xa.run(sys.argv[1:])
  
