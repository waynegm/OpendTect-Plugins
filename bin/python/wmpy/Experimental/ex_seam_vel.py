# Compute vertical average velocity for the SEAM I interpreter challenge data
#
# Copyright (C) 2024 Wayne Mogg All rights reserved.
#
# This file may be used under the terms of the MIT License
# (https://github.com/waynegm/OpendTect-External-Attributes/blob/master/LICENSE)
#
# Author:		Wayne Mogg
# Date: 		Mar, 2024
# Homepage:		http://waynegm.github.io/OpendTect-Plugin-Docs/External_Attributes/ExternalAttributes/
#
# Input: Single trace seismic data
# Output: Envelope with smooth apparent polarity
#
# Based on Wang etal (2018) "Seismic modulation model and envelope inversion with smoother apparent polarity" 
import sys,os
import numpy as np
from dtaidistance import dtw
#
# Import the module with the I/O scaffolding of the External Attribute
#
sys.path.insert(0, '/home/wayne/Work/WMSeismicSolutions/dGB/Development/Build/bin/ODPlugins/7.0.0/bin/python/')
import wmpy.extattrib as xa
#
# The attribute parameters
#
xa.params = {
    'Inputs': ['Time Volume', 'Depth Volume'],
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
        timedata = np.array(xa.Input['Time Volume'][0,0,:], dtype='d')
        depdata =  np.array(xa.Input['Depth Volume'][0,0,:], dtype='d')
#
        dtwpath = dtw.warping_path(depdata, timedata, use_c=True)
        outdata = np.zeros(len(timedata), dtype='f')
        for r_c, c_c in dtwpath:
                outdata[c_c] = r_c/c_c*2000.0 if c_c!=0 else xa.undef 

#        outdata = np.array(dtw.warp(depdata, timedata, dtwpath)[0])
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
  

