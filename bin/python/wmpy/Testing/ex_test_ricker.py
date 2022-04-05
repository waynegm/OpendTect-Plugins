# Generate Ricker wavelet at trace mid-point for testing 
#
# Input: Any volume to set the trace length
# Output: Trace with single Ricker wavelet at it's mid-point
#
import sys,os
import numpy as np
import math
#
# Import the module with the I/O scaffolding of the External Attribute
#
sys.path.insert(0, os.path.join(sys.path[0], '..'))
import extattrib as xa

#
# The attribute parameters - keep what you need
#
xa.params = {
	'Inputs': ['Anything will do'],
	'Frequency (Hz)' : {'Type': 'Number', 'Value': 20},
	'Parallel' : False,
	'Help'  : 'http://waynegm.github.io/OpendTect-Plugin-Docs/Attributes/ExternalAttrib/'
}
#
# Define the compute function
#
def doCompute():
	angle = xa.params['Angle (deg)']['Value']
	factor = math.tan( angle/180 * math.pi )
	while True:
		xa.doInput()
		indata = xa.Input['Intercept'][0,0,:]
		outdata = indata * factor
		xa.Output = outdata
		xa.doOutput()

xa.doCompute = doCompute
#
# Do it
#
xa.run(sys.argv[1:])


