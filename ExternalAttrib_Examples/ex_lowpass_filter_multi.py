#!/usr/bin/python

#
# Simple Butterworth LowPass Filter using Numpy/Scipy
# for the OpendTect ExternalAttrib plugin
#
import sys
import numpy as np
import scipy.signal as sig
#
# Import the module with the I/O scaffolding of the External Attribute
#
import extattrib as xa

#
# Set the attribute parameters
#
xa.params = {
	'Input': 'Filter Input',
	'ZSampMargin' : {'Value': [-30,30]},
	'Par_0' : {'Name': 'Filter Cutoff', 'Value': 40},
	'Par_1' : {'Name': 'Filter Order', 'Value': 3},
	'Parallel' : True,
	'Help'  : 'https://gist.github.com/waynegm/ed83d99c088db5cb37a9'
}
#
# Define the compute function
#
def doCompute():
	order = xa.params['Par_1']['Value']
	nyquist = 1.0/(2.0*xa.SI['zstep'])
	cutoff = xa.params['Par_0']['Value']/nyquist
	b, a = sig.butter(order, cutoff, 'low', analog=False)
	while True:
		xa.doInput()
		xa.Output = sig.filtfilt(b, a, xa.Input[0,0,:], padtype=None, padlen=0)
		xa.doOutput()

#
# Assign the compute function to the attribute
#
xa.doCompute = doCompute
#
# Do it
#
xa.run(sys.argv[1:])
  

