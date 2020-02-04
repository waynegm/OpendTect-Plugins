# External Attribute Skeleton
#
# Input: Single trace, single attribute
# Output: Multi-attribute
#
import sys,os
import numpy as np
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
	'Output': ['Output1', 'Output2', 'Output3'],
	'ZSampMargin' : {'Value': [-30,30], 'Symmetric': True},
	'Par_0' : {'Name': 'Parameter 0', 'Value': 0},
	'Par_1' : {'Name': 'Parameter 1', 'Value': 1},
	'Par_2' : {'Name': 'Parameter 2', 'Value': 2},
	'Par_3' : {'Name': 'Parameter 3', 'Value': 3},
	'Par_4' : {'Name': 'Parameter 4', 'Value': 4},
	'Par_5' : {'Name': 'Parameter 5', 'Value': 5},
	'Select' : {'Name': 'Option', 'Values': ['First', 'Second', 'Third'], 'Selection': 0},
	'Parallel' : False,
	'Help'  : 'http://waynegm.github.io/OpendTect-Plugin-Docs/Attributes/ExternalAttrib/'
}
#
# Define the compute function
#
def doCompute():
#
#	Initialise some constants from the attribute parameters or the SeismicInfo, xa.SI, array for use in the calculations
#	These are just some examples - keep/add what you need
#
	nyquist = 1.0/(2.0*xa.SI['zstep'])
	par0 = xa.params['Par_0']['Value']
	zw = xa.params['ZSampMargin']['Value'][1] - xa.params['ZSampMargin']['Value'][0] + 1
	select = xa.params['Select']['Selection']
#
#	This is the trace processing loop
#
	while True:
		xa.doInput()
#
#	After doInput the TraceInfo, xa.TI, array contains information specific to this trace segment - keep what you need
#
		number_of_samples = xa.TI['nrsamp']
		start_time =	xa.TI['z0']
		current_inline = xa.TI['inl']
		current_crossline = xa.TI['crl']
#
#	Get the input
#
		indata = xa.Input['Input'][0,0,:]
#
#	Your attribute calculation goes here
#
		out_1 = indata
		out_2 = indata * 2.0
		out_3 = indata * 3.0
#------------------------------------------------------------------------------------
#
#	Replace Output1-Output3 with the names in the xa.params Output array
#
		xa.Output['Output1'] = out_1
		xa.Output['Output2'] = out_2
		xa.Output['Output3'] = out_3
		xa.doOutput()

#
# Assign the compute function to the attribute
#
xa.doCompute = doCompute
#
# Do it
#
xa.run(sys.argv[1:])
  

