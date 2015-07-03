#!/usr/bin/python
#
# Local Polynomial Approximation Smoother using Numba JIT Acceleration - single thread version
#
# Smooths the data by fitting a 2nd order polynomial to a small window around 
# each data sample using gaussian weighted least squares. This implementation uses the Numba JIT to
# accelerate the convolution.
#
import sys
import numpy as np
from numba import jit,double
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
	'Parallel': False,
	'Help': 'http://waynegm.github.io/OpendTect-Plugin-Docs/External_Attributes/LPA_Attributes/'
}
#
# Define the compute function
#
def doCompute():
	dz = xa.params['ZSampMargin']['Value'][1] - xa.params['ZSampMargin']['Value'][0] + 1
	kernel = lpa3D_init(xa.SI['nrinl'], xa.SI['nrcrl'], dz, xa.params['Par_0']['Value'])[0]
	while True:
		xa.doInput()
		xa.Output = sconvolve(xa.Input, kernel)
		xa.doOutput()
	

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
	return DB.reshape((10,xs,ys,zs))
#
# Convolution of 3D filter with 3D data - only calulates the output for the centre trace
# Numba JIT used to accelerate the calculations

@jit(double(double[:,:,:], double[:,:,:]))
def sconvolve(arr, filt):
	X,Y,Z = arr.shape
	Xf,Yf,Zf = filt.shape
	X2 = X//2
	Y2 = Y//2
	Xf2 = Xf//2
	Yf2 = Yf//2
	Zf2 = Zf//2
	result = np.zeros(Z)
	for i in range(Zf2, Z-Zf2):
		num = 0.0
		for ii in range(Xf):
			for jj in range(Yf):
				for kk in range(Zf):
					num += (filt[Xf-1-ii, Yf-1-jj, Zf-1-kk] * arr[X2-Xf2+ii, Y2-Yf2+jj, i-Zf2+kk])
		result[i] = num
	return result

#
# Assign the compute function to the attribute
#
xa.doCompute = doCompute
#
# Do it
#
xa.run(sys.argv[1:])
  
