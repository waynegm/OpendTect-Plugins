# Orientation from Orientation Tensor by Local Polynomial Approximation using Numba JIT Acceleration
#
# Calculates the orientation using an orientation tensor derived from the coefficients of a local polynomial
# approximation (3D 2nd order polynomial using gaussian weighted least squares) as proposed by Farneback.
# 
# The orientation, based on the the dominant eigenvector, is returned as inline and crossline dip in ms/m or mm/m 
# depending on the if the Z axis is time or depth. This version includes filtering to remove dips outside a user
# specified range and smoothing by a Z axis lowpass filter.
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
	'Output': ['Crl_dip', 'Inl_dip'],
	'ZSampMargin' : {'Value':[-1,1], 'Symmetric': True},
	'StepOut' : {'Value': [1,1]},
	'Par_0': {'Name': 'Weight Factor', 'Value': 0.2},
	'Par_1': {'Name': '+/- Dip Clip(ms/m,mm/m)', 'Value': 300},
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
	maxDip = xa.params['Par_1']['Value']
	kernel = lpa3D_init(xs, ys, zs, wf)
	gam = 1/(8*((min(xs,ys,zs)-1)*wf)**2)
	inlFactor = xa.SI['zstep']/xa.SI['inldist'] * xa.SI['dipFactor']
	crlFactor = xa.SI['zstep']/xa.SI['crldist'] * xa.SI['dipFactor']
	smb,sma = butter(4, 0.3, btype='lowpass', analog=False)
	while True:
		xa.doInput()
		r = np.zeros((10,xa.TI['nrsamp']))
		for i in range(0,10):
			r[i,:] = xl.sconvolve(xa.Input,kernel[i])
		A = np.rollaxis(np.array([[r[4],r[7]/2, r[8]/2], [r[7]/2, r[5], r[9]/2], [r[8]/2, r[9]/2, r[6]]]),2)
		AAT = np.einsum('...ij,...jk->...ik', A, np.swapaxes(A,1,2))
		B = np.rollaxis(np.array([[r[1]],[r[2]],[r[3]]]),2)
		BBT = np.einsum('...ij,...jk->...ik', B, np.swapaxes(B,1,2))
		T = AAT+gam*BBT
		evals, evecs = np.linalg.eigh(T)
		ndx = evals.argsort()[:,-1]
		evecs = evecs[np.arange(0,T.shape[0],1),:,ndx]
		dip = -evecs[:,1]/evecs[:,2]*crlFactor
		dip[dip>maxDip] = 0.0
		dip[dip<-maxDip] = 0.0
		xa.Output['Crl_dip'] = filtfilt(smb, sma, dip)
		dip = -evecs[:,0]/evecs[:,2]*inlFactor
		dip[dip>maxDip] = 0.0
		dip[dip<-maxDip] = 0.0
		xa.Output['Inl_dip'] = filtfilt(smb, sma, dip)
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
  
