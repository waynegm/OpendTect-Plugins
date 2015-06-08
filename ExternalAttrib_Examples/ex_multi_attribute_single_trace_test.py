#!/usr/bin/python
#
# Simple test of single trace multi-attribute input
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
	'Inputs': ['A','B'],
	'Output': ['A','B','A-B']
}
#
# Define the compute function
#
def doCompute():
	while True:
		xa.doInput()
		xa.Output['A'] = xa.Input['A']
		xa.Output['B'] = xa.Input['B']
		xa.Output['A-B'] = xa.Input['A'] - xa.Input['B']
		xa.doOutput()
	

#
# Assign the compute function to the attribute
#
xa.doCompute = doCompute
#
# Do it
#
xa.run(sys.argv[1:])
  
