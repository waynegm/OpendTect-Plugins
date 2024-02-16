# Apply Adaptive Histogram Equalization
#
# Input: Data to equalize
# Output: Equalized data
#
import sys,os
import numpy as np
from skimage import exposure
from scipy import signal
#
# Import the module with the I/O scaffolding of the External Attribute
#
sys.path.insert(0, os.path.join(sys.path[0], '..'))
import extattrib as xa

#
# The attribute parameters - keep what you need
#
xa.params = {
	'Inputs': ['Input'],
	'ZSampMargin' : {'Value': [-75,75], 'Minimum': [-1,1], 'Symmetric': True, 'Hidden': False},
	'StepOut' : {'Value': [5,5], 'Minimum': [1,1], 'Hidden': False},
	'Clip Limit (0 to 1)' : {'Type': 'Number', 'Value': 0.5},
  	'Parallel' : False,
	'Help'  : xa.HelpRoot+'histogram_equalization/'
}
#
# Define the compute function
#
def doCompute():
#
#	Initialise some constants from the attribute parameters or the SeismicInfo, xa.SI, array for use in the calculations
#	These are just some examples - keep/add what you need
#
	centre_trace_x = xa.SI['nrinl']//2
	centre_trace_y = xa.SI['nrcrl']//2
	zw = xa.params['ZSampMargin']['Value'][1] - xa.params['ZSampMargin']['Value'][0] + 1
	kernel = np.array([xa.SI['nrinl'], xa.SI['nrcrl'], zw])
	clip_limit = xa.params['Clip Limit (0 to 1)']['Value']
	nyquist = 1.0/(2.0*xa.SI['zstep'])
	b, a = signal.butter(2, 2.0/nyquist, 'high', analog=False)
#
#	This is the trace processing loop
#
	while True:
		xa.doInput()
#
#	Get the input
#
		indata = xa.Input['Input']
#
#
#
		posdata = np.maximum(indata,0.0)
		negdata = np.minimum(indata,0.0)
		minval = np.min(negdata)
		maxval = np.max(posdata)
		posdata /= maxval
		negdata /= minval
		aheposdata = exposure.equalize_adapthist(posdata, kernel_size=kernel, clip_limit=clip_limit)
		ahenegdata = exposure.equalize_adapthist(negdata, kernel_size=kernel, clip_limit=clip_limit)
		outdata = aheposdata[centre_trace_x, centre_trace_y, :].astype(np.float32) * maxval +  \
					ahenegdata[centre_trace_x, centre_trace_y, :].astype(np.float32) * minval

#------------------------------------------------------------------------------------
#
		xa.Output = signal.filtfilt(b, a, outdata, padtype=None, padlen=0)
		xa.doOutput()

#
# Assign the compute function to the attribute
#
xa.doCompute = doCompute
#
# Do it
#
xa.run(sys.argv[1:])
  

