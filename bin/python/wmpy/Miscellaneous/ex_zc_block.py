#
# Zero Crossing Block
#
# Block the seismic trace between the zero crossings. Block amplitude equal to min/max in interval blocked
#
# Copyright (C) 2016 Wayne Mogg All rights reserved.
#
# This file may be used under the terms of the MIT License
# (https://github.com/waynegm/OpendTect-External-Attributes/blob/master/LICENSE)
#
# Author:		Wayne Mogg
# Date: 		August, 2016
# Homepage:		http://waynegm.github.io/OpendTect-Plugin-Docs/External_Attributes/ExternalAttributes/
#
import sys,os
import numpy as np
from numba import jit
#
# Import the module with the I/O scaffolding of the External Attribute
#
sys.path.insert(0, os.path.join(sys.path[0], '..'))
import extattrib as xa
#
# These are the attribute parameters
#
xa.params = {
	'Inputs': ['Input'],
	'ZSampMargin' : {'Value':[-20,20], 'Hidden': True},
	'Parallel' : False,
	'Help': 'http://waynegm.github.io/WMPlugin-Docs/docs/externalattributes/zcblock/'
}
#
# Define the compute function
#
def doCompute():
	while True:
		xa.doInput()

		inp = xa.Input['Input'][0,0,:]
		outp = np.zeros(inp.shape)

		response( inp, outp)
#
#	Get the output
		xa.Output = outp
		xa.doOutput()
#
# Square wave a trace
#
@jit(nopython=True)
def response(inp, outp):
    ns = inp.shape[0]
    start = 0
    pos = inp[0]>0
    for i in range(ns):
        if inp[i]<=0 and pos:
            if start<i-1:
                outp[start:i-1] = np.max(inp[start:i-1])
            else:
                outp[start] = inp[start]
            pos = False
            start = i
        if inp[i]>=0 and not pos:
            if start<i-1:
                outp[start:i-1] = np.min(inp[start:i-1])
            else:
                outp[start] = inp[start]
            pos=True
            start = i
    if pos:
        if start<ns-1:
            outp[start:ns] = np.max(inp[start:ns])
        else:
            outp[start] = inp[start]
    else:
        if start<ns-1:
            outp[start:ns] = np.min(inp[start:ns])
        else:
            outp[start] = inp[start]

#
# Assign the compute function to the attribute
#
xa.doCompute = doCompute
#
# Do it
#
xa.run(sys.argv[1:])
