# Generate 1D Synthetic Seismic data for Acoustic Impedance step model 
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
# Input: Single trace seismic data (only used for sampling and size)
# Output: Output replaced by 1D logAI model, smoother background logAI model or 
#         reflectivity model filtered by supplied wavelet 
#
import sys,os
import numpy as np
from scipy.signal import filtfilt
import pylops
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
    'Output': ['Impedance','Background','Seismic'],
    'Wavelet' : {'Type': 'File', 'Value': 'Seismics/*.wvlt'},
    'Background Smoother (samples)': {'Type': 'Number', 'Value': 100},
    'Parallel' : False
}
#
# Define the compute function
#
def doCompute():
#
#	Initialise some constants from the attribute parameters or the SeismicInfo, xa.SI, array
#
    dt = xa.SI['zstep']
    nsmooth = np.int(xa.params['Background Smoother (samples)']['Value'])
    waveletfile = xa.params['Wavelet']['Value']
#
# Load the wavelet
#
    wavelet = np.loadtxt(waveletfile, comments='!', skiprows=9)
#
#	This is the trace processing loop
#
    start = True
    logAI = []
    logAIbackground = []
    model = []
    while True:
        xa.doInput()
#
#	After doInput the TraceInfo, xa.TI, array contains information specific to this trace segment - keep what you need
#
        ns = xa.TI['nrsamp']
        if start:
	        start = False
	        vp = 1500 + np.arange(ns) + filtfilt(np.ones(5)/5., 1, np.random.normal(0,100,ns))
	        rho = 1000 + vp + filtfilt(np.ones(5)/5., 1, np.random.normal(0,30,ns))
	        vp[ns//2:] += 500
	        rho[ns//2:] += 100
	        logAI = np.log(vp*rho)
	        logAIbackground = filtfilt(np.ones(nsmooth)/float(nsmooth), 1, logAI)
	        PPop = pylops.avo.poststack.PoststackLinearModelling(wavelet/2, nt0=ns)
	        model = PPop * logAI
#
#	Get the input
#
        indata = xa.Input['Input'][0,0,:]
#
#	Prepare the output
#
        xa.Output['Impedance'] = logAI
        xa.Output['Background'] = logAIbackground
        xa.Output['Seismic'] = model
        xa.doOutput()

#
# Assign the compute function to the attribute
#
xa.doCompute = doCompute
#
# Do it
#
xa.run(sys.argv[1:])
  

