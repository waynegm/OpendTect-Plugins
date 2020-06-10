#
# Local cross-correlation to measure relative time delay between two inputs 
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
import extnumba as xn
#
# These are the attribute parameters
#
xa.params = {
	'Inputs': ['Reference', 'Match'],
	'Output': ['Shift', 'Quality'],
	'ZSampMargin' : {'Value':[-10,10], 'Symmetric': True},
	'Par_0': {'Name': 'Max Lag (samples)', 'Value': 5},
	'Help': 'http://waynegm.github.io/OpendTect-Plugin-Docs/external_attributes/Z_Delay_Est.html'
}
#
# Define the compute function
#
def doCompute():
	zw = xa.params['ZSampMargin']['Value'][1] - xa.params['ZSampMargin']['Value'][0] + 1
	nlag = int(xa.params['Par_0']['Value'])
	while True:
		xa.doInput()

		ref = xa.Input['Reference'][0,0,:]
		mat = xa.Input['Match'][0,0,:]
		ns = ref.shape[0]
		lag = np.zeros(ns)
		qual = np.zeros(ns)

		localCorr( ref, mat, zw, nlag, lag, qual)
#
#	Get the output
		xa.Output['Shift'] = lag*xa.SI['zstep']
		xa.Output['Quality'] = qual
		xa.doOutput()
#
# Local correlation - numba implementation
#
@jit(nopython=True)
def localCorr( reference, match, winlen, nlag, lag, qual ):
    hwin = winlen//2
    lags = 2*nlag+1
    ns = reference.shape[0]
    hxw = hwin-nlag
    cor = np.zeros(lags)
    refSSQ = np.zeros(ns)
    matSSQ = np.zeros(ns)
    xn.winSSQ(reference,2*hxw+1,refSSQ)
    xn.winSSQ(match,2*hxw+1,matSSQ)
    for ir in range(hwin,ns-hwin):
        rbeg = ir - hxw
        rend = ir + hxw + 1
        mbeg = rbeg - nlag
        mend = rend + nlag
        for il in range(lags):
            lbeg = rbeg + il - nlag
            lend = lbeg + 2 * hxw + 1
            sum = 0.0
            for iref,imat in zip(range(rbeg,rend),range(lbeg,lend)):
                sum += reference[iref]*match[imat]
            den = refSSQ[ir]*matSSQ[lbeg+hxw]
            if den== 0.0:
                cor[il] = 0.0
            else:
                cor[il] = sum/den
        pos = np.argmax(cor)
        if pos>0 and pos<lags-1:
            cp = (cor[pos-1]-cor[pos+1])/(2.*cor[pos-1]-4.*cor[pos]+2.*cor[pos+1])
            lag[ir] = pos-nlag+cp
            qual[ir] = max(min(cor[pos],1),0.)
        else:
            lag[ir]=0.0
            qual[ir]=0.0

#
# Assign the compute function to the attribute
#
xa.doCompute = doCompute
#
# Do it
#
xa.run(sys.argv[1:])
  
