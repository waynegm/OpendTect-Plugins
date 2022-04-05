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
xa.params = {
    'Inputs': ['Input'],
    'Parallel' : False
}
def ricker(t, f=20, loc=1.0):
    ft2 = (np.pi*f*(t-loc))**2
    wav = (1.-2.*ft2)*np.exp(-ft2)
    wav = wav/np.max(wav)
    return wav
#
def doCompute():
#
    dt = xa.SI['zstep']
#
    while True:
        xa.doInput()
#
        ns = xa.TI['nrsamp']
        start_time =	xa.TI['z0']
#
        indata = xa.Input['Input'][0,0,:]
#
        t = np.arange(0, ns*dt, dt)
        outdata = np.zeros(ns)
        outdata += scipy.signal.chirp(t, f0=35, t1=t[int(ns*0.6)], f1=35, method='linear')*0.2
        s = scipy.signal.chirp(t, f0=15, t1=t[int(ns*0.6)], f1=15, method='linear')*0.1
        s[int(ns*0.6):] = 0.0
        outdata += s
        s = scipy.signal.chirp(t, f0=15, t1=t[int(ns*0.4)], f1=80.0, method='quadratic')*0.1
        s[int(ns*0.4):] = 0.0
        outdata += np.roll(s, int(ns*0.6))
#        s = 0.4*np.cos(130*np.pi*t+2*np.sin(40*np.pi*t))
#        s[int(ns*0.4):] = 0.0
#        outdata += np.roll(s, int(ns*0.1))
        outdata += ricker(t, 50, t[int(ns/2)])*0.7
#        outdata += ricker(t, 40, t[int(ns/2)+10])*-0.5
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


