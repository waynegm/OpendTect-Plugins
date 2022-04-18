#!/usr/bin/python
#
# Calculate attributes from Inline and Crossline dip
#
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
	'Inputs': ['Crl_dip', 'Inl_dip'],
	'Output': ['True Dip', 'Dip Azimuth'],
	'Help': 'http://waynegm.github.io/WMPlugin-Docs/docs/externalattributes/dipazimuth/'
}
#
# Define the compute function
#
def doCompute():
	inlFactor = xa.SI['zstep']/xa.SI['inldist'] * xa.SI['dipFactor']
	crlFactor = xa.SI['zstep']/xa.SI['crldist'] * xa.SI['dipFactor']
	while True:
		xa.doInput()

#
#	Get the output
		xa.Output['True Dip'] = np.sqrt(xa.Input['Crl_dip']*xa.Input['Crl_dip']+xa.Input['Inl_dip']*xa.Input['Inl_dip'])
		xa.Output['Dip Azimuth'] = np.degrees(np.arctan2(xa.Input['Inl_dip'],xa.Input['Crl_dip']))
		xa.doOutput()
#
# Assign the compute function to the attribute
#
xa.doCompute = doCompute
#
# Do it
#
xa.run(sys.argv[1:])

