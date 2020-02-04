# Add gaussian noise to an input
#
# Copyright (C) 2016 Wayne Mogg All rights reserved.
#
# This file may be used under the terms of the MIT License
# (https://github.com/waynegm/OpendTect-External-Attributes/blob/master/LICENSE)
#
# Author:		Wayne Mogg
# Date: 		September, 2016
# Homepage:		http://waynegm.github.io/OpendTect-Plugin-Docs/External_Attributes/ExternalAttributes/
#
# Input: Single trace seismic data
# Output: Seismic data with added gaussian noise
#
import sys,os
import numpy as np
#
# Import the module with the I/O scaffolding of the External Attribute
#
sys.path.insert(0, os.path.join(sys.path[0], '..'))
import extattrib as xa
#
# The attribute parameters
#
xa.params = {
	'Inputs': ['Input'],
	'ZSampMargin' : {'Value': [-5,5], 'Hidden': True, 'Symmetric': True},
	'Par_0' : {'Name': 'S/N Ratio', 'Value': 1},
	'Parallel' : True,
	'Help'  : 'http://waynegm.github.io/OpendTect-Plugin-Docs/external_attributes/Add_Noise.html'
}
#
# Define the compute function
#
def doCompute():
#
#	Initialise some constants from the attribute parameters 
#
	zw = xa.params['ZSampMargin']['Value'][1] - xa.params['ZSampMargin']['Value'][0] + 1
#
#	This is the trace processing loop
#
	while True:
		xa.doInput()
		data = xa.Input['Input'][0,0,:]
#
#   Compute noise
#
		vardata  = np.var(data)
		noise = np.random.randn(data.shape[-1])
		varnoise = np.var(noise)
		scale = vardata/(varnoise*xa.params['Par_0']['Value'])
#
#	Output
#
		xa.Output = data + scale*noise
		xa.doOutput()
#
# Assign the compute function to the attribute
#
xa.doCompute = doCompute
#
# Do it
#
xa.run(sys.argv[1:])
  

