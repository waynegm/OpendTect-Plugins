#
# Python External Attribute Numba Function Library
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
import numpy as np
from numba import jit
import math

#
# Moving window average of a 1D array
@jit(nopython=True, cache=True)
def winMean( inp, winlen, outp):
	ns = inp.shape[0]
	hw = winlen//2
	hwp1 = hw+1
	sum = 0.0
	for i in range(hw):
		sum += inp[i]
		
	for i in range(hwp1):
		sum += inp[i+hw]
		outp[i] = sum / (hwp1+i)
		
	for i in range(hw+1,ns-hw):
		sum += inp[i+hw] - inp[i-hw-1]
		outp[i] = sum / winlen
		
	for i in range(ns-hw,ns):
		sum -= inp[i-hw-1]
		outp[i] = sum / (ns-i+hw)
		
#
# Moving window sum of a 1D array
@jit(nopython=True, cache=True)
def winSum( inp, winlen, outp):
	ns = inp.shape[0]
	hw = winlen//2
	hwp1 = hw+1
	sum = 0.0
	for i in range(hw):
		sum += inp[i]
		
	for i in range(hwp1):
		sum += inp[i+hw]
		outp[i] = sum
		
	for i in range(hw+1,ns-hw):
		sum += inp[i+hw] - inp[i-hw-1]
		outp[i] = sum
		
	for i in range(ns-hw,ns):
		sum -= inp[i-hw-1]
		outp[i] = sum
		
#
# Moving window sum of squares of a 1D array
@jit()
def winSSQ( inp, winlen, outp):
	inpsq = np.square(inp)
	ns = inp.shape[0]
	hw = winlen//2
	hwp1 = hw+1
	sum = 0.0
	for i in range(hw):
		sum += inpsq[i]
		
	for i in range(hwp1):
		sum += inpsq[i+hw]
		outp[i] = math.sqrt(sum)
		
	for i in range(hw+1,ns-hw):
		sum += inpsq[i+hw] - inpsq[i-hw-1]
		outp[i] = math.sqrt(sum)
		
	for i in range(ns-hw,ns):
		sum -= inpsq[i-hw-1]
		outp[i] = math.sqrt(sum)
	
	
