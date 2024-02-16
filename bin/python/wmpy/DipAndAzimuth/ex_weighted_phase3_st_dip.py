#!/usr/bin/python
#
# Inline, Crossline, True dip, Dip Azimuth or Coherency using the 3D complex trace envelope weighted phase structure tensor
# Derivatives calulated using Kroon's 3 point filter
#
import sys,os
import numpy as np

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
	'Output': ['Crl_dip', 'Inl_dip', 'True Dip', 'Dip Azimuth', 'Cplane'],
	'ZSampMargin' : {'Value':[-15,15], 'Minimum':[-2,2], 'Symmetric': True},
	'StepOut' : {'Value': [2,2], 'Minimum': [2,2], 'Symmetric': True},
	'Par_0' : {'Name': 'Tensor ZStepOut', 'Value': 1},
	'Par_1' : {'Name': 'Band', 'Value': 0.9},
	'Help': xa.HelpRoot+'dipazimuth/#orientation-using-the-envelope-weighted-3d-complex-trace-phase-structure-tensor'
}
#
# Define the compute function
#
def doCompute():
	xs = xa.SI['nrinl']
	ys = xa.SI['nrcrl']
	zs = min(2*int(xa.params['Par_0']['Value'])+1,3)
	kernel = xl.getGaussian(xs-2, ys-2, zs-2)
	hxs = xs//2
	hys = ys//2
	inlFactor = xa.SI['zstep']/xa.SI['inldist'] * xa.SI['dipFactor']
	crlFactor = xa.SI['zstep']/xa.SI['crldist'] * xa.SI['dipFactor']
	N = xa.params['ZSampMargin']['Value'][1]
	band = xa.params['Par_1']['Value']
	hilbkernel = xl.hilbert_kernel(N, band)
	while True:
		xa.doInput()

		indata = xa.Input['Input']
#
#	Analytic Signal
#
		ansig = np.apply_along_axis(np.convolve,-1, indata, hilbkernel, mode="same")
		sr = np.real(ansig)
		si = np.imag(ansig)
#
#	Compute partial derivatives
#
		sx = xl.kroon3( sr, axis=0 )
		sy = xl.kroon3( sr, axis=1 )
		sz = xl.kroon3( sr, axis=2 )
		shx = xl.kroon3( si, axis=0 )
		shy = xl.kroon3( si, axis=1 )
		shz = xl.kroon3( si, axis=2 )

		px = sr[1:xs-1,1:ys-1,:] * shx[1:xs-1,1:ys-1,:] - si[1:xs-1,1:ys-1,:] * sx[1:xs-1,1:ys-1,:]
		py = sr[1:xs-1,1:ys-1,:] * shy[1:xs-1,1:ys-1,:] - si[1:xs-1,1:ys-1,:] * sy[1:xs-1,1:ys-1,:]
		pz = sr[1:xs-1,1:ys-1,:] * shz[1:xs-1,1:ys-1,:] - si[1:xs-1,1:ys-1,:] * sz[1:xs-1,1:ys-1,:]
#
#	Inner product of gradients
		px2 = px * px
		py2 = py * py
		pz2 = pz * pz
		pxpy = px * py
		pxpz = px * pz
		pypz = py * pz
#
# Outer smoothing
		rgx2 = xl.sconvolve(px2, kernel)
		rgy2 = xl.sconvolve(py2, kernel)
		rgz2 = xl.sconvolve(pz2, kernel)
		rgxgy = xl.sconvolve(pxpy, kernel)
		rgxgz = xl.sconvolve(pxpz, kernel)
		rgygz = xl.sconvolve(pypz, kernel)
#
#	Form the structure tensor
		T = np.rollaxis(np.array([	[rgx2,  rgxgy, rgxgz],
									[rgxgy, rgy2,  rgygz],
									[rgxgz, rgygz, rgz2 ]]), 2)
#
#	Get the eigenvalues and eigen vectors and calculate the dips
		evals, evecs = np.linalg.eigh(T)
		ndx = evals.argsort()
		evec = evecs[np.arange(0,T.shape[0],1),:,ndx[:,2]]
		e1 = evals[np.arange(0,T.shape[0],1),ndx[:,2]]
		e2 = evals[np.arange(0,T.shape[0],1),ndx[:,1]]

		xa.Output['Crl_dip'] = -evec[:,1]/evec[:,2]*crlFactor
		xa.Output['Inl_dip'] = -evec[:,0]/evec[:,2]*inlFactor
		xa.Output['True Dip'] = np.sqrt(xa.Output['Crl_dip']*xa.Output['Crl_dip']+xa.Output['Inl_dip']*xa.Output['Inl_dip'])
		xa.Output['Dip Azimuth'] = np.degrees(np.arctan2(xa.Output['Inl_dip'],xa.Output['Crl_dip']))
		xa.Output['Cplane'] = (e1-e2)/(e1+e2)

		xa.doOutput()


#
# Assign the compute function to the attribute
#
xa.doCompute = doCompute
#
# Do it
#
xa.run(sys.argv[1:])

