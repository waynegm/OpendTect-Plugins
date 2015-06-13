#!/usr/bin/python
#
# Simple test of multi trace multi-attribute input - output mean
#
#
import sys
import numpy as np
#
# Import the module with the I/O scaffolding of the External Attribute
#
import extattrib as xa

#
# These are the attribute parameters
#
xa.params = {
	'Inputs': ['A','B','C'],
	'Output': ['meanA','meanB','meanC'],
	'StepOut': {'Value': [1,1]}
}
#
# Define the compute function
#
def doCompute():
	while True:
		xa.doInput()
		xa.Output['meanA'] = np.mean(xa.Input['A'], axis=(0,1))
		xa.Output['meanB'] = np.mean(xa.Input['B'], axis=(0,1))
		xa.Output['meanC'] = np.mean(xa.Input['C'], axis=(0,1))
		xa.doOutput()
	

#
# Assign the compute function to the attribute
#
xa.doCompute = doCompute
#
# Do it
#
xa.run(sys.argv[1:])
  
