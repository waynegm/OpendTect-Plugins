# Generate complex chirp signal for testing spectral decomposition 
#
# Copyright (C) 2021 Wayne Mogg All rights reserved.
#
# This file may be used under the terms of the MIT License
# (https://github.com/waynegm/OpendTect-External-Attributes/blob/master/LICENSE)
#
# Author:		Wayne Mogg
# Date: 		May, 2021
# Homepage:		http://waynegm.github.io/OpendTect-Plugin-Docs/External_Attributes/ExternalAttributes/
#
# Input: Single trace seismic data
# Output: Output replaced by chirp signal
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
# The attribute parameters - keep what you need
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
#	Initialise some constants from the attribute parameters or the SeismicInfo, xa.SI, array for use in the calculations
#	These are just some examples - keep/add what you need
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
        s = scipy.signal.chirp(t, f0=20.0, t1=t[-50], f1=80.0, method='quadratic')*0.5
        s[ns-50:] = 0.0
        outdata += np.roll(s, 25)
        s = scipy.signal.chirp(t, f0=60.0, t1=t[50], f1=60.0, method='linear')*0.75
        s[100:] = 0.0
        outdata += np.roll(s, 50)
        s= scipy.signal.chirp(t, f0=40.0, t1=t[250], f1=10.0, method='linear')*0.5
        s[250:] = 0.0
        outdata += np.roll(s,100)
        s = scipy.signal.chirp(t, f0=80, t1=t[-50], f1=80, method='linear')*.25
        s[-50:] = 0.0
        outdata += np.roll(s, 25)
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
  

