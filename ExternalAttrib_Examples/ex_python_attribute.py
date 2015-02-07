#!/usr/bin/python

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
	'Input': 'Test Input',
	'Output': ['Output_1', 'Output_2'],
	'ZSampMargin' : [-10,10],
	'StepOut' : [1,1],
	'Par_0' : {'Name': 'First Parameter', 'Value' : 100.0},
	'Par_1' : {'Name': 'Second Parameter','Value': 200.0}
}
#
# This is where the attribute calculation takes place 
#
# Global numpy record xa.SI contains following parameters:
#		nrtraces	number of traces in Input array
#		nrinl		number of inlines in data block
#		nrcrl		number of crosslines in data block
#		zstep		trace sample interval
#		inldist		distance between inlines
#		crldist		distance between crosslines
#
# Global numpy record xa.TI contains following parameters:
#		nrsamp		number of samples per trace in the data block
#		z0			starting time/depth of the data block
#		inl			current inline
#		crl			current crossline
#
# NumPy array xa.Input contains the trace data as a 3D array with shape (xa.SI['nrinl'], xa.SI['nrcrl'], xa.TI['nrsamp'])
# Dictionary xa.Output contains a 1D array of size xa.TI['nrsamp'] for for each of the outputs listed in xa.paramsis written to stdout
#

def doCompute():
#
# index of current trace position in Input numpy array
#
	ilndx = xa.SI['nrinl']//2
	crldx = xa.SI['nrcrl']//2
	while True:
		xa.doInput()
		xa.Output['Output_1'] = xa.Input[ilndx,crldx,:]
		xa.Output['Output_2'] = -xa.Input[ilndx,crldx,:]
		xa.doOutput()
	
#
# Assign the compute function to the attribute
#
xa.doCompute = doCompute
#
# Do it
#
xa.run(sys.argv[1:])
