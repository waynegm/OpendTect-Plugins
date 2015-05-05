#!/usr/bin/python
#
# Local Polynomial Approximation Smoother
#
# Smooths the data by fitting a 2nd order polynomial to a small window around 
# each data sample using gaussian weighted least squares. This implementation uses
# the SciPy.ndimage convolve function. Checkout ex_lpa_smooth.py for the exact same 
# algorithm accelerated by the Numba JIT compiler.
#
import sys
import numpy as np
from scipy.ndimage import convolve
#
# Import the module with the I/O scaffolding of the External Attribute
#
import extattrib as xa

#
# These are the attribute parameters
#
xa.params = {
	'Input': 'Input',
	'ZSampMargin' : {'Value':[-1,1], 'Symmetric': True},
	'StepOut' : {'Value': [1,1]},
	'Par_0': {'Name': 'Weight Factor', 'Value': 0.2},
	'Help': 'http://waynegm.github.io/OpendTect-Plugin-Docs/External-Attributes/LPA-Smoothing/'
}
#
# Define the compute function
#
def doCompute():
#
# index of current trace position in Input numpy array
#
	ilndx = xa.SI['nrinl']//2
	crldx = xa.SI['nrcrl']//2
	dz = xa.params['ZSampMargin']['Value'][1] - xa.params['ZSampMargin']['Value'][0] + 1
	kernel = lpa3D_smooth(xa.SI['nrinl'], xa.SI['nrcrl'], dz, xa.params['Par_0']['Value'])
	while True:
		xa.doInput()
		xa.Output = convolve(xa.Input, kernel)[ilndx,crldx,:]
		xa.doOutput()
	
#
# Get Smoothing kernel
#
def lpa3D_smooth( xs, ys, zs, sigma=0.2 ):
	DB = lpa3D_init( xs, ys, zs, sigma )
	return DB[0].reshape((xs,ys,zs))
#
# Find the LPA solution for a 2nd order polynomial in 3D
#
def lpa3D_init( xs, ys, zs, sigma=0.2 ):
	std = sigma * (min(xs,ys,zs)-1)
	hxs = (xs-1)/2
	hys = (ys-1)/2
	hzs = (zs-1)/2
	xtmp = np.linspace(-hxs,hxs,xs)
	ytmp = np.linspace(-hys,hys,ys)
	ztmp = np.linspace(-hzs,hzs,zs)
	xyz = np.meshgrid(xtmp,ytmp,ztmp, indexing='ij')
	x = xyz[0].flatten()
	y = xyz[1].flatten()
	z = xyz[2].flatten()
	w = np.exp(-(x**2+y**2+z**2)/(2*std**2))
	W = np.diagflat(w)
	A = np.dstack((np.ones(x.size), x, y, z, x*x, y*y, z*z, x*y, x*z, y*z)).reshape((x.size,10))
	DB = np.linalg.inv(A.T.dot(W).dot(A)).dot(A.T).dot(W)
	return DB
#
# Assign the compute function to the attribute
#
xa.doCompute = doCompute
#
# Do it
#
xa.run(sys.argv[1:])
  
