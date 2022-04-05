# Generate signals for testing Spectral Decomposition - monochromatic
# 
# Based on first synthetic example in Liu etal (2016)
# "Applications of variational mode decomposition in seismic time-frequency analysis"
#
# Copyright (C) 2022 Wayne Mogg All rights reserved.
#
# This file may be used under the terms of the MIT License
# (https://github.com/waynegm/OpendTect-External-Attributes/blob/master/LICENSE)
#
# Author:		Wayne Mogg
# Date: 		Feb, 2022
# Homepage:		http://waynegm.github.io/OpendTect-Plugin-Docs/External_Attributes/ExternalAttributes/
#
# Input: Single trace seismic data
# Output: Output replaced by test signal
#
import sys,os
import numpy as np
import scipy.signal
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
    'Parallel' : False
}
#
# Define the compute function
#
def doCompute():
#
    dt = xa.SI['zstep']
#
#	This is the trace processing loop
#
    while True:
        xa.doInput()
#
#	After doInput the TraceInfo, xa.TI, array contains information specific to this trace segment - keep what you need
#
        ns = xa.TI['nrsamp']
        nshalf = int(ns/2)
        start_time =	xa.TI['z0']
#
#	Get the input
#
        indata = xa.Input['Input'][0,0,:]
#
#	Your attribute calculation goes here
#
        t = np.linspace(0, ns*dt, ns)
        outdata = np.zeros(ns)
        s1 = np.cos(10.0*np.pi*t+10.0*np.pi*np.square(t))
        s2 = np.cos(60.0*np.pi*t)
        s2[nshalf:] = 0.0
        s3 = np.cos(120.0*np.pi*t-10.0*np.pi)
        s3[:nshalf] = 0.0
        outdata = s1 + s2 + s3
#------------------------------------------------------------------------------------
#
        xa.Output = outdata
        xa.doOutput()

#
# Assign the compute function to the attribute
#
xa.doCompute = doCompute
#
# Do it
#
xa.run(sys.argv[1:])
  

