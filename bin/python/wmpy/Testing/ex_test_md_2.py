# Generate signals for testing Spectral Decomposition 
# 
# Based on second synthetic example in Liu etal (2016)
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
def morlet(t,f=20, dt=0.02, loc=1.0):
    wav = np.exp(-(2*np.sqrt(np.log(2))*(t-loc)/dt)**2) * np.cos(2*np.pi*f*(t-loc))
    wav = wav/np.max(wav)
    return wav

def ricker(t, f=20, loc=1.0):
    ft2 = (np.pi*f*(t-loc))**2
    wav = (1.-2.*ft2)*np.exp(-ft2)
    wav = wav/np.max(wav)
    return wav
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
        s1 = np.cos(40.0*np.pi*t)
        s1[int(1300/2000*ns):int(1700/2000*ns)] = 0.0
        s2 = np.cos(14.0*np.pi*t)
        s2[0:int(1300/2000*ns)] = 0.0
        s2[int(1700/2000*ns):] = 0.0
        s3 = np.cos(60.0*np.pi*t)
        s3[0:int(1300/2000*ns)] = 0.0
        s3[int(1500/2000*ns):] = 0.0
        s4 = np.cos(80.0*np.pi*t)
        s4[0:int(1500/2000*ns)] = 0.0
        s4[int(1700/2000*ns):] = 0.0
        s5 = ricker(t,50, 0.5)
        s6 = ricker(t,30, 1.0)
        s7 = morlet(t, 100,0.05,0.3)
        outdata = s1 + s2 + s3 + s4 + s5 + s6 + s7
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
  

