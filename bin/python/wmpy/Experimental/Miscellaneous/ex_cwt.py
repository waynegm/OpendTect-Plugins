# Miscellaneous CWT output
#
import sys,os
import numpy as np
from scipy import signal
# Import the module with the I/O scaffolding of the External Attribute
#
sys.path.insert(0, os.path.join(sys.path[0], '..', '..'))
import extattrib as xa
import extlib as xl

#
# These are the attribute parameters
#
xa.params = {
	'Inputs': ['Input'],
	'Output': ['Find Peaks'],
	'ZSampMargin' : {'Value':[-8,8], 'Symmetric': True},
	'Parallel': False
}
#
# Define the compute function
#
def doCompute():
	zw = xa.params['ZSampMargin']['Value'][1] - xa.params['ZSampMargin']['Value'][0] + 1
	width = np.arange(1,zw)
	while True:
		xa.doInput()
		
		data  = xa.Input['Input'][0,0,:]
#
# Find signal peaks
		peakidx = signal.find_peaks_cwt(data, width)
		out = np.zeros(data.shape)
		out[peakidx] = data[peakidx]
		xa.Output['Find Peaks'] = out
#
# Output
		xa.doOutput()
	
#
# Assign the compute function to the attribute
#
xa.doCompute = doCompute
#
# Do it
#
xa.run(sys.argv[1:])
  
