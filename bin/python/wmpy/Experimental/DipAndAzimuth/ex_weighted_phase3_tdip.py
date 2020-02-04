#
# Inline, Crossline, True dip or Dip Azimuth using the envelope weighted phase of the complex trace
# using tensor orientation and Kroon's 3 point derivative filter
#
import sys,os
import numpy as np
from scipy.signal import hilbert
import scipy.ndimage as ndi
#
# Import the module with the I/O scaffolding of the External Attribute
#
sys.path.insert(0, os.path.join(sys.path[0], '..','..'))
import extattrib as xa
import extlib as xl

#
# These are the attribute parameters
#
xa.params = {
	'Inputs': ['Input'],
	'Output': ['Crl_dip', 'Inl_dip', 'True Dip', 'Dip Azimuth'],
	'ZSampMargin' : {'Value':[-1,1], 'Hidden': True},
	'StepOut' : {'Value': [1,1], 'Hidden': True}
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

		s = xa.Input['Input']
		sh = np.imag( hilbert(s) ) 
#
#	Compute partial derivatives
		sx = xl.kroon3( s, axis=0 )[hxs,hys,:]
		sy = xl.kroon3( s, axis=1 )[hxs,hys,:]
		sz = xl.kroon3( s, axis=2 )[hxs,hys,:]
		shx = xl.kroon3( sh, axis=0 )[hxs,hys,:]
		shy = xl.kroon3( sh, axis=1 )[hxs,hys,:]
		shz = xl.kroon3( sh, axis=2 )[hxs,hys,:]
		
		px = s[hxs,hys,:] * shx - sh[hxs,hys,:] * sx
		py = s[hxs,hys,:] * shy - sh[hxs,hys,:] * sy
		pz = s[hxs,hys,:] * shz - sh[hxs,hys,:] * sz
#
#	Form the structure tensor
		px2 = px * px
		py2 = py * py
		pz2 = pz * pz
		pxpy = px * py
		pxpz = px * pz
		pypz = py * pz
		T = np.rollaxis(np.array([	[px2,  pxpy, pxpz],
									[pxpy, py2,  pypz],
									[pxpz, pypz, pz2 ]]), 2)
#
#	Get the eigenvalues and eigen vectors and calculate the dips
		evals, evecs = np.linalg.eigh(T)
		ndx = evals.argsort()
		evec = evecs[np.arange(0,T.shape[0],1),:,ndx[:,-1]]
		xa.Output['Crl_dip'] = -evec[:,1]/evec[:,2]*crlFactor
		xa.Output['Inl_dip'] = -evec[:,0]/evec[:,2]*inlFactor
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
  
