#
# Calculate Orientation attributes from pre-calculated gradients using a tensor
# Inputs: Inline, Crossline and Z gradients
# Outputs: Inline, Crossline, True dip, Dip Azimuth
#
import sys,os
import numpy as np

#
# Import the module with the I/O scaffolding of the External Attribute
#
sys.path.insert(0, os.path.join(sys.path[0], '..','..'))
import extattrib as xa

#
# These are the attribute parameters
#
xa.params = {
	'Inputs': ['In-line gradient', 'Cross-line gradient', 'Z gradient'],
	'Output': ['Crl_dip', 'Inl_dip', 'True Dip', 'Dip Azimuth']
}
#
# Define the compute function
#
def doCompute():
	inlFactor = xa.SI['zstep']/xa.SI['inldist'] * xa.SI['dipFactor']
	crlFactor = xa.SI['zstep']/xa.SI['crldist'] * xa.SI['dipFactor']
	while True:
		xa.doInput()

		gx = xa.Input['In-line gradient'][0,0,:]
		gy = xa.Input['Cross-line gradient'][0,0,:]
		gz = xa.Input['Z gradient'][0,0,:]
#
#	Inner product of  gradients
		gx2 = gx * gx
		gy2 = gy * gy
		gz2 = gz * gz
		gxgy = gx * gy
		gxgz = gx * gz
		gygz = gy * gz
#
#	Form the structure tensor
		T = np.rollaxis(np.array([	[gx2,  gxgy, gxgz],
									[gxgy, gy2,  gygz],
									[gxgz, gygz, gz2 ]]), 2)
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
  
