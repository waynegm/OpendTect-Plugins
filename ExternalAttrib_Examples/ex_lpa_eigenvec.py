#!/usr/bin/python
#
# Orientation Tensor by Local Polynomial Approximation using Numba JIT Acceleration
#
# Uses the coefficients of a local polynomial approximation (3D 2nd order polynomial using 
# gaussian weighted least squares) to calculate the orientation tensor proposed by Farneback.
#
# The numbering of the outputs refer to the corresponding eigenvalue.
#
import sys
import numpy as np
from scipy.ndimage import convolve
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
	'Output': ['t1x', 't1y', 't1z', 't2x', 't2y', 't2z', 't3x', 't3y', 't3z'],
	'ZSampMargin' : {'Value':[-1,1], 'Symmetric': True},
	'StepOut' : {'Value': [1,1]},
	'Par_0': {'Name': 'Weight Factor', 'Value': 0.2},
	'Help': 'http://waynegm.github.io/OpendTect-Plugin-Docs/External_Attributes/LPA_Attributes/'
}
#
# Define the compute function
#
def doCompute():
	xs = xa.SI['nrinl']
	ys = xa.SI['nrcrl']
	zs = xa.params['ZSampMargin']['Value'][1] - xa.params['ZSampMargin']['Value'][0] + 1
	wf = xa.params['Par_0']['Value']
	kernel = lpa3D_init(xs, ys, zs, wf)
	gam = 1/(8*((min(xs,ys,zs)-1)*wf)**2)
	while True:
		xa.doInput()
		r = np.zeros((10,xa.TI['nrsamp']))
		for i in range(0,10):
			r[i,:] = sconvolve(xa.Input,kernel[i])
		A = np.rollaxis(np.array([[r[4],r[7]/2, r[8]/2], [r[7]/2, r[5], r[9]/2], [r[8]/2, r[9]/2, r[6]]]),2)
		AAT = np.einsum('...ij,...jk->...ik', A, np.swapaxes(A,1,2))
		B = np.rollaxis(np.array([[r[1]],[r[2]],[r[3]]]),2)
		BBT = np.einsum('...ij,...jk->...ik', B, np.swapaxes(B,1,2))
		T = AAT+gam*BBT
		p = np.rollaxis(T,0,3)
		xa.Output['t1x'] = p[0,0,:]
		xa.Output['t1y'] = p[0,1,:]
		xa.Output['t1z'] = p[0,2,:]
		xa.Output['t2x'] = p[1,0,:]
		xa.Output['t2y'] = p[1,1,:]
		xa.Output['t2z'] = p[1,2,:]
		xa.Output['t3x'] = p[2,0,:]
		xa.Output['t3y'] = p[2,1,:]
		xa.Output['t3z'] = p[2,2,:]
		xa.doOutput()
	
#
# Find the LPA solution for a 2nd order polynomial in 3D
#
def lpa3D_init( xs, ys, zs, sigma=0.2 ):
	sx = sigma * (xs-1)
	sy = sigma * (ys-1)
	sz = sigma * (zs-1)
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
	w = np.exp(-(x**2/(2*sx**2) + y**2/(2*sy**2) + z**2/(2*sz**2)))
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
  
