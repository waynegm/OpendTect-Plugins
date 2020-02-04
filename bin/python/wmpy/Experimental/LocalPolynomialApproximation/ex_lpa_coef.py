# Local Polynomial Approximation using Numba JIT Acceleration
#
# Calculates the coefficients of a 3D 2nd order polynomial approximation to a small window around 
# each data sample using gaussian weighted least squares. This implementation uses the Numba JIT to
# accelerate the convolution.
# 
# The approximation has the following form:
# r0 + r1*x + r2*y + r3*z + r4*x^2 + r5*y^2 +r6*z^2 + r7*x*y + r8*xz + r9*yz
# where x,y and z are relative to the analysis location, ie the analysis location has x=y=z=0.
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
	'Input': 'Input',
	'Output': ['r0', 'r1', 'r2', 'r3', 'r4', 'r5', 'r6', 'r7', 'r8', 'r9'],
	'ZSampMargin' : {'Value':[-1,1], 'Symmetric': True},
	'StepOut' : {'Value': [1,1]},
	'Par_0': {'Name': 'Weight Factor', 'Value': 0.2},
	'Help': 'http://waynegm.github.io/OpendTect-Plugin-Docs/External_Attributes/LPA_Attributes/'
}
#
# Define the compute function
#
def doCompute():
	zs = xa.params['ZSampMargin']['Value'][1] - xa.params['ZSampMargin']['Value'][0] + 1
	kernel = lpa3D_init(xa.SI['nrinl'], xa.SI['nrcrl'], zs, xa.params['Par_0']['Value'])
	while True:
		xa.doInput()
		for i in range(0,10):
			xa.Output['r'+str(i)] = xl.sconvolve(xa.Input,kernel[i])
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
# Assign the compute function to the attribute
#
xa.doCompute = doCompute
#
# Do it
#
xa.run(sys.argv[1:])
  
