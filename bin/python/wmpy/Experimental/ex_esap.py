# Compute the Envelope with Smoother Apparent Polarity
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
# Output: Envelope with smooth apparent polarity
#
# Based on Wang etal (2018) "Seismic modulation model and envelope inversion with smoother apparent polarity" 
import sys,os
import numpy as np
import scipy.interpolate as si
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
        envelope = np.abs(analytic_signal)
        locmax, _ = ss.find_peaks(envelope, height=0)
        nsap = locmax.size + 2
        sap_x = np.zeros(nsap)
        sap_y = np.zeros(nsap)
        sap_x[0] = z_start
        sap_x[1:-1] = z[locmax]
        sap_x[-1] = z[-1]
        as_real = np.real(analytic_signal[locmax])
        as_imag = np.imag(analytic_signal[locmax])
        sap_y[1:-1] = np.sign(np.where(np.absolute(as_real)>np.absolute(as_imag), as_real, as_imag))
#        sap_y[1:-1] = np.sign(as_real)
        sap_y[sap_y==0.0] = 1.0
        sap_y[0] = 0.0
        sap_y[-1] = 0.0
        sapfunc = si.PchipInterpolator(sap_x, sap_y)
        sap = sapfunc(z)
        outdata = sap * envelope
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
  

