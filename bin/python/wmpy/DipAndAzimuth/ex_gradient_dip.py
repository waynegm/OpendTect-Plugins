#
# Calculate Orientation attributes from pre-calculated gradients
# Inputs: Inline, Crossline and Z gradients
# Outputs: Inline, Crossline, True dip, Dip Azimuth
#
import sys,os
import numpy as np

#
# Import the module with the I/O scaffolding of the External Attribute
#
sys.path.insert(0, os.path.join(sys.path[0], '..'))
import extattrib as xa

#
# These are the attribute parameters
#
xa.params = {
	'Inputs': ['In-line gradient', 'Cross-line gradient', 'Z gradient'],
	'Output': ['Crl_dip', 'Inl_dip', 'True Dip', 'Dip Azimuth'],
	'Help': xa.HelpRoot+'dipazimuth/#orientation-from-gradients'
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
# Calculate the dips and output
		xa.Output['Crl_dip'] = -gy/gz*crlFactor
		xa.Output['Inl_dip'] = -gx/gz*inlFactor
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

