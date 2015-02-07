#!/usr/bin/python
#
# Calculate inline and crossline dip using the operator proposed by Xudong Jiang in
# "Extracting image orientation feature by using integration operator" 
# Pattern Recognition 40 (2007) 705-717
#
import sys
from math import *
import numpy as np
import sympy as sp
from scipy.ndimage import convolve
from scipy.signal import gaussian
from scipy.signal import medfilt
#
# Import the module with the I/O scaffolding of the External Attribute
#
import extattrib as xa

#
# These are the attribute parameters
#
xa.params = {
	'Input': 'Input',
	'Output': ['Crl_dip', 'Inl_dip'],
	'ZSampMargin' : [-10,10],
	'StepOut' : [1,1],
	'Par_0': {'Name': 'Gaussian Smoother Std Deviation', 'Value': 1}
}
#
# Define the compute function
#
def doCompute():
	inl_kernel = xukernel(xa.SI['nrcrl'].item(), xa.params['Par_0']['Value'])
	crl_kernel = xukernel(xa.SI['nrinl'].item(), xa.params['Par_0']['Value'])
#
# index of current trace position in Input numpy array
#
	ilndx = xa.SI['nrinl']//2
	crldx = xa.SI['nrcrl']//2
	while True:
		xa.doInput()
		xa.Output['Crl_dip'] = xudip(inl_kernel, xa.Input[ilndx,:,:])
		xa.Output['Inl_dip'] = xudip(crl_kernel, xa.Input[:,crldx,:])
		xa.doOutput()
	

#
# Apply the orientation operator kernel
#
def xudip(kernel, inarr):
    sz = np.shape(kernel)[0]
    M2 = sz//2
    r = convolve(inarr, kernel.T)
    i = convolve(inarr, kernel)
    res = np.where(r[M2,:] != 0.0, -i[M2,:]/r[M2,:], 0.0)
    
    return res

#
# Compute the orientation operator kernel
#
def xukernel(size, smooth):
    s = sp.Symbol('s',integer=True)
    b = sp.Symbol('b', integer=True)
    _M = sp.Symbol('M',integer=True)
    _mp = sp.Symbol('mp', integer=True)
    p = sp.Symbol('p')
    p = (sp.Product(s-b, (b,0,_M))/(s-_mp)).doit()
    res = np.zeros((size, size))
    M = size-1
    M2 = M//2
    for n in range(1,M2+1):
        nmm = M2 - n
        npp = M2 + n
        for m in range(0,M2+1):
            mmm = M2 - m
            mpp = M2 + m
            const = (-1.0)**mmm/(factorial(mmm)*factorial(mpp))
            v = p.subs(_M,M).subs(_mp,mpp)
            res[npp,mpp] = const*sp.re(sp.N(sp.integrate(v,(s,nmm,npp))))
            res[npp,mmm] = res[npp,mpp]
            res[nmm,mpp] = -res[npp,mpp]
            res[nmm,mmm] = res[nmm,mpp]
    w = gaussian(size, smooth).reshape((size,1))
    return w*res/np.sum(w)

#
# Assign the compute function to the attribute
#
xa.doCompute = doCompute
#
# Do it
#
xa.run(sys.argv[1:])
  
