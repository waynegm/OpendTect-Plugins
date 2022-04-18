#
# Inline, Crossline, True dip or Dip Azimuth using the 3D complex trace phase
# using derivatives calculated with Kroon's 3 point filter
#
import sys,os
import numpy as np
import scipy.signal as ss
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
	'Output': ['Crl_dip', 'Inl_dip', 'True Dip', 'Dip Azimuth'],
	'ZSampMargin' : {'Value':[-15,15], 'Symmetric': True},
	'Par_0' : {'Name': 'Band', 'Value': 0.9},
	'StepOut' : {'Value': [1,1], 'Hidden': True},
	'Parallel': True,
	'Help': 'http://waynegm.github.io/WMPlugin-Docs/docs/externalattributes/dipazimuth/#orientation-from-the-3d-complex-trace-phase'
}
#
# Define the compute function
#
def doCompute():
	hxs = xa.SI['nrinl']//2
	hys = xa.SI['nrcrl']//2
	inlFactor = xa.SI['zstep']/xa.SI['inldist'] * xa.SI['dipFactor']
	crlFactor = xa.SI['zstep']/xa.SI['crldist'] * xa.SI['dipFactor']
	band = xa.params['Par_0']['Value']
	N = xa.params['ZSampMargin']['Value'][1]
	kernel = xl.hilbert_kernel(N, band)
	while True:
		xa.doInput()

		indata = xa.Input['Input']
#
#	Analytic Signal
#
		ansig = np.apply_along_axis(np.convolve,-1, indata, kernel, mode="same")
		sr = np.real(ansig)
		si = np.imag(ansig)
#
#	Compute partial derivatives
		sx = xl.kroon3( sr, axis=0 )[hxs,hys,:]
		sy = xl.kroon3( sr, axis=1 )[hxs,hys,:]
		sz = xl.kroon3( sr, axis=2 )[hxs,hys,:]
		shx = xl.kroon3( si, axis=0 )[hxs,hys,:]
		shy = xl.kroon3( si, axis=1 )[hxs,hys,:]
		shz = xl.kroon3( si, axis=2 )[hxs,hys,:]

		px = sr[hxs,hys,:] * shx - si[hxs,hys,:] * sx
		py = sr[hxs,hys,:] * shy - si[hxs,hys,:] * sy
		pz = sr[hxs,hys,:] * shz - si[hxs,hys,:] * sz
#
#	Calculate dips and output
		xa.Output['Crl_dip'] = -py/pz*crlFactor
		xa.Output['Inl_dip'] = -px/pz*inlFactor
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

