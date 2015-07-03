#!/usr/bin/python
#
# Inline/Crossline dip from Gradient Energy Tensor
#
# Calculates the orientation using the gradient enery tensor.
# 
# The orientation is returned based on the dominant eigenvector.
#
import sys
import numpy as np
import scipy.ndimage as ndi
#
# Import the module with the I/O scaffolding of the External Attribute
#
import extattrib as xa

#
# These are the attribute parameters
#
xa.params = {
	'Input': 'Input',
	'Output': ['Crl_dip', 'Inl_dip'],
	'ZSampMargin' : {'Value':[-2,2], 'Hidden': True},
	'StepOut' : {'Value': [2,2], 'Hidden': True},
	'Help': 'http://waynegm.github.io/OpendTect-Plugin-Docs/External-Attributes/GET-Attributes/'
}
#
# Define the compute function
#
def doCompute():
	hxs = xa.SI['nrinl']//2
	hys = xa.SI['nrcrl']//2
	inlFactor = xa.SI['zstep']/xa.SI['inldist'] * xa.SI['dipFactor']
	crlFactor = xa.SI['zstep']/xa.SI['crldist'] * xa.SI['dipFactor']
	while True:
		xa.doInput()

		gx = scharr3(xa.Input, axis=0)
		gy = scharr3(xa.Input, axis=1)
		gz = scharr3(xa.Input, axis=2)

		gxx = scharr3(gx, axis=0)
		gyy = scharr3(gy, axis=1)
		gzz = scharr3(gz, axis=2)
		gxy = scharr3(gy, axis=0)
		gxz = scharr3(gz, axis=0)
		gyz = scharr3(gz, axis=1)

		laplace = gxx + gyy + gzz
		gx3 = scharr3(laplace,axis=0)
		gy3 = scharr3(laplace,axis=1)
		gz3 = scharr3(laplace,axis=2)

		H = np.array([	[gxx[hxs,hys,:],gxy[hxs,hys,:],gxz[hxs,hys,:]],
						[gxy[hxs,hys,:],gyy[hxs,hys,:],gyz[hxs,hys,:]],
						[gxz[hxs,hys,:],gyz[hxs,hys,:],gzz[hxs,hys,:]]])
		dG = np.array([[gx[hxs,hys,:]],[gy[hxs,hys,:]],[gz[hxs,hys,:]]])
		Tg = np.array([[gx3[hxs,hys,:]],[gy3[hxs,hys,:]],[gz3[hxs,hys,:]]])

		T = np.zeros((3,3))
		res = np.zeros((3,xa.Input.shape[2]))
		for i in range(xa.Input.shape[2]):
			T = H[:,:,i].dot(H[:,:,i].T) - 0.5 * (dG[:,:,i].dot(Tg[:,:,i].T) + Tg[:,:,i].dot(dG[:,:,i].T))
			evals,evecs = np.linalg.eigh(T)
			ndx = evals.argsort()[2]
			res[:,i] = evecs[:,ndx]

		xa.Output['Crl_dip'] = -res[1,:]/res[2,:]*crlFactor
		xa.Output['Inl_dip'] = -res[0,:]/res[2,:]*inlFactor

		xa.doOutput()
	
#
# Scharr 3 pont derivative filter
#
def scharr3( input, axis=-1, output=None, mode="reflect", cval=0.0):
	"""Calculate a size 3 Scharr derivative filter.
	Parameters
	----------
	%(input)s
	%(axis)s
	%(output)s
	%(mode)s
	%(cval)s
	"""
	input = np.asarray(input)
	axis = ndi._ni_support._check_axis(axis, input.ndim)
	output, return_value = ndi._ni_support._get_output(output, input)
	ndi.correlate1d(input, [-0.5, 0, 0.5], axis, output, mode, cval, 0)
	axes = [ii for ii in range(input.ndim) if ii != axis]
	for ii in axes:
		ndi.correlate1d(output, [0.12026,0.75948,0.12026], ii, output, mode, cval, 0,)
	return return_value
#
# Assign the compute function to the attribute
#
xa.doCompute = doCompute
#
# Do it
#
xa.run(sys.argv[1:])
  
