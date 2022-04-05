# Compute the real or imaginary component of the Analytic Signal
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
# Output: Either the real or imaginary component of the analytic signal
#
import sys,os
import numpy as np
import scipy.signal as ss
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
    'Output': ['Real', 'Imaginary'],
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
        ns = xa.TI['nrsamp']
        z_start =	xa.TI['z0']
        indata = xa.Input['Input'][0,0,:]
        z = np.arange(z_start,z_start+ns*dt, dt)
#
        analytic_signal = ss.hilbert(indata)
#
        xa.Output['Real'] = np.real(analytic_signal)
        xa.Output['Imaginary'] = np.imag(analytic_signal)
        xa.doOutput()

#
# Assign the compute function to the attribute
#
xa.doCompute = doCompute
#
# Do it
#
xa.run(sys.argv[1:])
  

