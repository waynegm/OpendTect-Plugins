#
# Python External Attribute Library
#
# Copyright (C) 2016 Wayne Mogg All rights reserved.
#
# This file may be used under the terms of the MIT License
# (https://github.com/waynegm/OpendTect-External-Attributes/blob/master/LICENSE)
#
# Author:		Wayne Mogg
# Date: 		March, 2016
# Homepage:		http://waynegm.github.io/OpendTect-Plugin-Docs/External_Attributes/ExternalAttributes/
#
import numpy as np
import scipy.ndimage as ndi
import scipy.signal as ss
from numba import jit, double


#
#
def getOutput(output, input, shape=None):
    if shape is None:
        shape = input.shape
    if output is None:
        output = np.zeros(shape, dtype=input.dtype.name)
    elif type(output) in [type(type), type(np.zeros((4,)).dtype)]:
        output = np.zeros(shape, dtype=output)
    elif isinstance(output, string_types):
        output = np.typeDict[output]
        output = np.zeros(shape, dtype=output)
    elif output.shape != shape:
        raise RuntimeError("output shape not correct")
    return output

# 
#
def hilbert_kernel(N, band=0.9):
	"""Generate a Hilbert Transform kernel.
	
	Calculate a 2*N+1 length complex kernel to compute a Hilbert Transform as
	per:
	Turner, 2009: An efficient analytic signal generator
	
	Args:
		N: the half-length of the transform kernel.
		band: optional, specifies the bandwidth of the transform
			in normalised frequency where 1.0 is the nyquist. Default is 0.9.

	Returns:
		A 2*N+1 long complex array with the kernel.
	"""
	x = np.linspace(-N,N,2*N+1) * np.pi /2.0 * 1j
	result = ss.firwin(2*N+1,band/2, window="nuttall") * np.exp(x) * 2
	return result

# 
#
def scharr3( input, axis=-1, output=None, mode="reflect", cval=0.0):
	"""Full block first derivative along an axis using a 3 point Scharr filter.
	
	Applies a 3 point Scharr first derivative filter along an axis of the input
	array as per: 
	Scharr, 2005: Optimal derivative filter families for transparent motion
	estimation.
	
	Args:
		input: the array to be filtered.
		axis: optional, specifies the array axis to calculate the
			derivative. Default is the last axis.
		output: optional, an array to store the derivative filter output.
			Should be the same shape as the input array.
		mode: {'reflect', 'constant', 'nearest', 'mirror', 'wrap'} optional,
			specifies how the array boundaries are filtered. Default is
			'reflect'.
		cval: optional, specified value to pad input array if mode is 
			'constant'. Default is 0.0.
			
	Returns:
		derivative filtered array with same shape as input. Note for each array
		dimension indices 1:-1 will be free of boundary effects. 
	"""
	input = np.asarray(input)
	axis = ndi._ni_support._check_axis(axis, input.ndim)
	output = getOutput(output, input)
	ndi.correlate1d(input, [-0.5, 0, 0.5], axis, output, mode, cval, 0)
	axes = [ii for ii in range(input.ndim) if ii != axis]
	for ii in axes:
		ndi.correlate1d(output, [0.12026,0.75948,0.12026], ii, output, mode, cval, 0)
	return output
#
#
def _separableFilterFull( input, weights, output=None, mode="reflect", cval=0.0):
	"""Apply separable filter to the input trace buffer, full block output.
	
	Applies the filter described by weights. The input is assumed to be a
	NxMxNS (N & M>=3 and odd) block of traces. The function returns the filter
	output only over the full NxM block. A certain number of the outer traces
	of the output block, determined by the dimensions of the weights array
	will be contaminated by boundary effects.
	
	Args:
		input: the array to be filtered - NxM (N & M>=3 and odd)block of traces.
		weights: array with filter weights for each dimension of input
		output: optional, a 1D array to store the derivative filter output.
			Should be the same length as the last dimension of the input.
		mode: {'reflect', 'constant', 'nearest', 'mirror', 'wrap'} optional,
			specifies how the array boundaries are filtered. Default is
			'reflect'. Only applies along the last axis of the input
		cval: optional, specified value to pad input array if mode is 
			'constant'. Default is 0.0.

	Returns:
		filtered array with same shape as input. Note that the outer traces
		will be affected by boundary effects. 
	"""
	input = np.asarray(input)
	output = getOutput(output, input)
	ndi.correlate1d(input, weights[0], 0, output, mode, cval, 0)
	ndi.correlate1d(output, weights[1], 1, output, mode, cval, 0)
	if input.ndim==3 :
		ndi.correlate1d(output, weights[2], 2, output, mode, cval, 0)
	return output
#
#
def _separableFilterSingle( input, weights, output=None, mode="reflect", cval=0.0):
	"""Apply separable filter to the input trace buffer, single trace output.
	
	Applies the filter described by weights. The input is assumed to be a
	NxMxNS (N & M>=3 and odd) block of traces. The function returns the filter
	output only at the centre trace of the NxM block.
	
	Args:
		input: the array to be filtered - NxM (N & M>=3 and odd)block of traces.
		weights: array with filter weights for each dimension of input
		output: optional, a 1D array to store the derivative filter output.
			Should be the same length as the last dimension of the input.
		mode: {'reflect', 'constant', 'nearest', 'mirror', 'wrap'} optional,
			specifies how the array boundaries are filtered. Default is
			'reflect'. Only applies along the last axis of the input
		cval: optional, specified value to pad input array if mode is 
			'constant'. Default is 0.0.

	Returns:
		filtered 1D array with same length as last dimension of the input.
	"""
	input = np.asarray(input)
	inshape = input.shape
	output = getOutput(output, input,(inshape[-1]))

	if input.ndim==2 :
		W0 = weights[0]
		W1 = weights[1]
		n = np.int_(inshape[0] - W1.shape[0])//2
		use = input if n==0 else input[n:-n,:]
		tmp0 = np.sum(W0[:,np.newaxis]*use, 0)
		ndi.correlate1d(tmp0, W1, -1, output, mode='reflect')
	elif input.ndim==3 :
		W0 = weights[0]
		W1 = weights[1]
		W2 = weights[2]
		n = np.int_(inshape[0] - W0.shape[0])//2
		m = np.int_(inshape[1] - W1.shape[0])//2
		use = input if n==0 else input[n:-n,:,:]
		use = use if m==0 else use[:,m:-m,:]
		tmp0 = np.sum(W0[:,np.newaxis,np.newaxis]*use, 0)
		tmp1 = np.sum(W1[:,np.newaxis]*tmp0, 0)
		ndi.correlate1d(tmp1, W2, -1, output, mode='reflect')
	return output
#
#
def scharr3_dx( input, output=None, full=True, mode="reflect", cval=0.0):
	"""Scharr 3 point first derivative along the 1st (x) axis of input.
	
	Applies a 3 point Scharr first derivative filter along the 1st (x) axis
	of the input array as per: 
	Scharr, 2005: Optimal derivative filter families for transparent motion
	estimation.
	The input is assumed to be a NxMxNS (N & M>=3 and odd) block of traces
	with NS samples per trace. The function returns the derivative only at
	the centre trace of the NxM block if full is False. If full is True the
	derivative is calculated for all traces but a certain number of the outer
	traces in the output, determined by the dimensions of the weights array,
	will be contaminated by boundary effects.
	
	Args:
		input: the array to be filtered - NxM (N & M>=3 and odd)block of traces.
		output: optional, a 1D array to store the derivative filter output.
			Should be the same length as the last dimension of the input.
		full: optional, boolean determining if derivative is calulated for all
			traces (True, the  default) or just the centre trace (False)
		mode: {'reflect', 'constant', 'nearest', 'mirror', 'wrap'} optional,
			specifies how the array boundaries are filtered. Default is
			'reflect'. Only applies along the last axis of the input
		cval: optional, specified value to pad input array if mode is 
			'constant'. Default is 0.0.
			
	Returns:
		for full==True: derivative filtered array the same shape as the input
		for full==False: derivative filtered 1D array with same length as last
						 dimension of the input.
	"""
	weights = np.array([
							[-0.5, 0, 0.5],
							[0.12026,0.75948,0.12026],
							[0.12026,0.75948,0.12026]
						])
	if full:
		return _separableFilterFull(input, weights, output, mode, cval)
	else:
		return _separableFilterSingle(input, weights, output, mode, cval)
#
#
def scharr3_dy( input, output=None, full=True, mode="reflect", cval=0.0):
	"""Scharr 3 point first derivative along the 2nd (y) axis of input.
	
	Applies a 3 point Scharr first derivative filter along the 2nd (y) axis
	of the input array as per: 
	Scharr, 2005: Optimal derivative filter families for transparent motion
	estimation.
	The input is assumed to be a NxMxNS (N & M>=3 and odd) block of traces
	with NS samples per trace. The function returns the derivative only at
	the centre trace of the NxM block if full is False. If full is True the
	derivative is calculated for all traces but a certain number of the outer
	traces in the output, determined by the dimensions of the weights array,
	will be contaminated by boundary effects.
	
	Args:
		input: the array to be filtered - NxM (N & M>=3 and odd)block of traces.
		output: optional, a 1D array to store the derivative filter output.
			Should be the same length as the last dimension of the input.
		full: optional, boolean determining if derivative is calulated for all
			traces (True, the  default) or just the centre trace (False)
		mode: {'reflect', 'constant', 'nearest', 'mirror', 'wrap'} optional,
			specifies how the array boundaries are filtered. Default is
			'reflect'. Only applies along the last axis of the input
		cval: optional, specified value to pad input array if mode is 
			'constant'. Default is 0.0.
			
	Returns:
		for full==True: derivative filtered array the same shape as the input
		for full==False: derivative filtered 1D array with same length as last
						 dimension of the input.
	"""
	weights = np.array([
							[0.12026,0.75948,0.12026],
							[-0.5, 0, 0.5],
							[0.12026,0.75948,0.12026]
						])
	if full:
		return _separableFilterFull(input, weights, output, mode, cval)
	else:
		return _separableFilterSingle(input, weights, output, mode, cval)
#
#
def scharr3_dz( input, output=None, full=True, mode="reflect", cval=0.0):
	"""Scharr 3 point first derivative along the last (z) axis of input.
	
	Applies a 3 point Scharr first derivative filter along the last (z) axis
	of the input array as per: 
	Scharr, 2005: Optimal derivative filter families for transparent motion
	estimation.
	The input is assumed to be a NxMxNS (N & M>=3 and odd) block of traces
	with NS samples per trace. The function returns the derivative only at
	the centre trace of the NxM block if full is False. If full is True the
	derivative is calculated for all traces but a certain number of the outer
	traces in the output, determined by the dimensions of the weights array,
	will be contaminated by boundary effects.
	
	Args:
		input: the array to be filtered - NxM (N & M>=3 and odd)block of traces.
		output: optional, a 1D array to store the derivative filter output.
			Should be the same length as the last dimension of the input.
		full: optional, boolean determining if derivative is calulated for all
			traces (True, the  default) or just the centre trace (False)
		mode: {'reflect', 'constant', 'nearest', 'mirror', 'wrap'} optional,
			specifies how the array boundaries are filtered. Default is
			'reflect'. Only applies along the last axis of the input
		cval: optional, specified value to pad input array if mode is 
			'constant'. Default is 0.0.
			
	Returns:
		for full==True: derivative filtered array the same shape as the input
		for full==False: derivative filtered 1D array with same length as last
						 dimension of the input.
	"""
	weights = np.array([
							[0.12026,0.75948,0.12026],
							[0.12026,0.75948,0.12026],
							[-0.5, 0, 0.5]
						])
	if full:
		return _separableFilterFull(input, weights, output, mode, cval)
	else:
		return _separableFilterSingle(input, weights, output, mode, cval)
#
#
def scharr3_dxx( input, output=None, full=True, mode="reflect", cval=0.0):
	"""Scharr 3 point second derivative along the 1st (x) axis of input.
	
	Applies a 3 point Scharr second derivative filter along the 1st (x) axis
	of the input array as per: 
	Scharr, 2005: Optimal derivative filter families for transparent motion
	estimation.
	The input is assumed to be a NxMxNS (N & M>=3 and odd) block of traces
	with NS samples per trace. The function returns the derivative only at
	the centre trace of the NxM block if full is False. If full is True the
	derivative is calculated for all traces but a certain number of the outer
	traces in the output, determined by the dimensions of the weights array,
	will be contaminated by boundary effects.
	
	Args:
		input: the array to be filtered - NxM (N & M>=3 and odd)block of traces.
		output: optional, a 1D array to store the derivative filter output.
			Should be the same length as the last dimension of the input.
		full: optional, boolean determining if derivative is calulated for all
			traces (True, the  default) or just the centre trace (False)
		mode: {'reflect', 'constant', 'nearest', 'mirror', 'wrap'} optional,
			specifies how the array boundaries are filtered. Default is
			'reflect'. Only applies along the last axis of the input
		cval: optional, specified value to pad input array if mode is 
			'constant'. Default is 0.0.
			
	Returns:
		for full==True: derivative filtered array the same shape as the input
		for full==False: derivative filtered 1D array with same length as last
						 dimension of the input.
	"""
	weights = np.array([
							[1, -2, 1],
							[0.21478,0.57044,0.21478],
							[0.21478,0.57044,0.21478]
						])
	if full:
		return _separableFilterFull(input, weights, output, mode, cval)
	else:
		return _separableFilterSingle(input, weights, output, mode, cval)
#
#
def scharr3_dyy( input, output=None, full=True, mode="reflect", cval=0.0):
	"""Scharr 3 point second derivative along the 2nd (y) axis of input.
	
	Applies a 3 point Scharr second derivative filter along the 2nd (y) axis
	of the input array as per: 
	Scharr, 2005: Optimal derivative filter families for transparent motion
	estimation.
	The input is assumed to be a NxMxNS (N & M>=3 and odd) block of traces
	with NS samples per trace. The function returns the derivative only at
	the centre trace of the NxM block if full is False. If full is True the
	derivative is calculated for all traces but a certain number of the outer
	traces in the output, determined by the dimensions of the weights array,
	will be contaminated by boundary effects.
	
	Args:
		input: the array to be filtered - NxM (N & M>=3 and odd)block of traces.
		output: optional, a 1D array to store the derivative filter output.
			Should be the same length as the last dimension of the input.
		full: optional, boolean determining if derivative is calulated for all
			traces (True, the  default) or just the centre trace (False)
		mode: {'reflect', 'constant', 'nearest', 'mirror', 'wrap'} optional,
			specifies how the array boundaries are filtered. Default is
			'reflect'. Only applies along the last axis of the input
		cval: optional, specified value to pad input array if mode is 
			'constant'. Default is 0.0.
			
	Returns:
		for full==True: derivative filtered array the same shape as the input
		for full==False: derivative filtered 1D array with same length as last
						 dimension of the input.
	"""
	weights = np.array([
							[0.21478,0.57044,0.21478],
							[1, -2, 1],
							[0.21478,0.57044,0.21478]
						])
	if full:
		return _separableFilterFull(input, weights, output, mode, cval)
	else:
		return _separableFilterSingle(input, weights, output, mode, cval)
#
#
def scharr3_dzz( input, output=None, full=True, mode="reflect", cval=0.0):
	"""Scharr 3 point second derivative along the last (z) axis of input.
	
	Applies a 3 point Scharr second derivative filter along the last (z) axis
	of the input array as per: 
	Scharr, 2005: Optimal derivative filter families for transparent motion
	estimation.
	The input is assumed to be a NxMxNS (N & M>=3 and odd) block of traces
	with NS samples per trace. The function returns the derivative only at
	the centre trace of the NxM block if full is False. If full is True the
	derivative is calculated for all traces but a certain number of the outer
	traces in the output, determined by the dimensions of the weights array,
	will be contaminated by boundary effects.
	
	Args:
		input: the array to be filtered - NxM (N & M>=3 and odd)block of traces.
		output: optional, a 1D array to store the derivative filter output.
			Should be the same length as the last dimension of the input.
		full: optional, boolean determining if derivative is calulated for all
			traces (True, the  default) or just the centre trace (False)
		mode: {'reflect', 'constant', 'nearest', 'mirror', 'wrap'} optional,
			specifies how the array boundaries are filtered. Default is
			'reflect'. Only applies along the last axis of the input
		cval: optional, specified value to pad input array if mode is 
			'constant'. Default is 0.0.
			
	Returns:
		for full==True: derivative filtered array the same shape as the input
		for full==False: derivative filtered 1D array with same length as last
						 dimension of the input.
	"""
	weights = np.array([
							[0.21478,0.57044,0.21478],
							[0.21478,0.57044,0.21478],
							[1, -2, 1]
						])
	if full:
		return _separableFilterFull(input, weights, output, mode, cval)
	else:
		return _separableFilterSingle(input, weights, output, mode, cval)
#
#
def scharr3_dxy( input, output=None, full=True, mode="reflect", cval=0.0):
	"""Scharr 3 point first derivative along the 1st and 2nd (x and y) axis of input.
	
	Applies a 3 point Scharr first derivative filter along the 1st and 2nd 
	(x and y) axis of the input array as per: 
	Scharr, 2005: Optimal derivative filter families for transparent motion
	estimation.
	The input is assumed to be a NxMxNS (N & M>=3 and odd) block of traces
	with NS samples per trace. The function returns the derivative only at
	the centre trace of the NxM block if full is False. If full is True the
	derivative is calculated for all traces but a certain number of the outer
	traces in the output, determined by the dimensions of the weights array,
	will be contaminated by boundary effects.
	
	Args:
		input: the array to be filtered - NxM (N & M>=3 and odd)block of traces.
		output: optional, a 1D array to store the derivative filter output.
			Should be the same length as the last dimension of the input.
		full: optional, boolean determining if derivative is calulated for all
			traces (True, the  default) or just the centre trace (False)
		mode: {'reflect', 'constant', 'nearest', 'mirror', 'wrap'} optional,
			specifies how the array boundaries are filtered. Default is
			'reflect'. Only applies along the last axis of the input
		cval: optional, specified value to pad input array if mode is 
			'constant'. Default is 0.0.
			
	Returns:
		for full==True: derivative filtered array the same shape as the input
		for full==False: derivative filtered 1D array with same length as last
						 dimension of the input.
	"""
	weights = np.array([
							[0.5, 0, -0.5],
							[0.5, 0, -0.5],
							[0.21478,0.57044,0.21478]
						])
	if full:
		return _separableFilterFull(input, weights, output, mode, cval)
	else:
		return _separableFilterSingle(input, weights, output, mode, cval)
#
#
def scharr3_dxz( input, output=None, full=True, mode="reflect", cval=0.0):
	"""Scharr 3 point first derivative along the 1st and last (x and z) axis of input.
	
	Applies a 3 point Scharr first derivative filter along the 1st and last 
	(x and z) axis of the input array as per: 
	Scharr, 2005: Optimal derivative filter families for transparent motion
	estimation.
	The input is assumed to be a NxMxNS (N & M>=3 and odd) block of traces
	with NS samples per trace. The function returns the derivative only at
	the centre trace of the NxM block if full is False. If full is True the
	derivative is calculated for all traces but a certain number of the outer
	traces in the output, determined by the dimensions of the weights array,
	will be contaminated by boundary effects.
	
	Args:
		input: the array to be filtered - NxM (N & M>=3 and odd)block of traces.
		output: optional, a 1D array to store the derivative filter output.
			Should be the same length as the last dimension of the input.
		full: optional, boolean determining if derivative is calulated for all
			traces (True, the  default) or just the centre trace (False)
		mode: {'reflect', 'constant', 'nearest', 'mirror', 'wrap'} optional,
			specifies how the array boundaries are filtered. Default is
			'reflect'. Only applies along the last axis of the input
		cval: optional, specified value to pad input array if mode is 
			'constant'. Default is 0.0.
			
	Returns:
		for full==True: derivative filtered array the same shape as the input
		for full==False: derivative filtered 1D array with same length as last
						 dimension of the input.
	"""
	weights = np.array([
							[0.5, 0, -0.5],
							[0.21478,0.57044,0.21478],
							[0.5, 0, -0.5]
						])
	if full:
		return _separableFilterFull(input, weights, output, mode, cval)
	else:
		return _separableFilterSingle(input, weights, output, mode, cval)
#
#
def scharr3_dyz( input, output=None, full=True, mode="reflect", cval=0.0):
	"""Scharr 3 point first derivative along the 2nd and last (y and z) axis of input.
	
	Applies a 3 point Scharr first derivative filter along the 2nd and last 
	(y and z) axis of the input array as per: 
	Scharr, 2005: Optimal derivative filter families for transparent motion
	estimation.
	The input is assumed to be a NxMxNS (N & M>=3 and odd) block of traces
	with NS samples per trace. The function returns the derivative only at
	the centre trace of the NxM block if full is False. If full is True the
	derivative is calculated for all traces but a certain number of the outer
	traces in the output, determined by the dimensions of the weights array,
	will be contaminated by boundary effects.
	
	Args:
		input: the array to be filtered - NxM (N & M>=3 and odd)block of traces.
		output: optional, a 1D array to store the derivative filter output.
			Should be the same length as the last dimension of the input.
		full: optional, boolean determining if derivative is calulated for all
			traces (True, the  default) or just the centre trace (False)
		mode: {'reflect', 'constant', 'nearest', 'mirror', 'wrap'} optional,
			specifies how the array boundaries are filtered. Default is
			'reflect'. Only applies along the last axis of the input
		cval: optional, specified value to pad input array if mode is 
			'constant'. Default is 0.0.
			
	Returns:
		for full==True: derivative filtered array the same shape as the input
		for full==False: derivative filtered 1D array with same length as last
						 dimension of the input.
	"""
	weights = np.array([
							[0.21478,0.57044,0.21478],
							[0.5, 0, -0.5],
							[0.5, 0, -0.5]
						])
	if full:
		return _separableFilterFull(input, weights, output, mode, cval)
	else:
		return _separableFilterSingle(input, weights, output, mode, cval)
#
#
def scharr3_Hessian( input, full=True, mode="reflect", cval=0.0):
	"""Hessian using Sharr 3 point derivative _separableFilterSingle

	The input is assumed to be a NxMxNS (N & M>=3 and odd) block of traces
	with NS samples per trace. The function returns the Hessian only at
	the centre trace of the NxM block if full is False. If full is True the
	Hessian is calculated for all traces but a certain number of the outer
	traces in the output, determined by the dimensions of the weights array,
	will be contaminated by boundary effects.
	
	Returns:
		for full=True: Hessian array the same shape as input
		for full=False: Hessian the same length as last dimension of the input.
	"""
	hxx = scharr3_dxx(input, None, full, mode, cval)
	hyy = scharr3_dyy(input, None, full, mode, cval)
	hzz = scharr3_dzz(input, None, full, mode, cval)
	hxy = scharr3_dxy(input, None, full, mode, cval)
	hxz = scharr3_dxz(input, None, full, mode, cval)
	hyz = scharr3_dyz(input, None, full, mode, cval)
	
	return np.array([[ hxx, hxy, hxz ],
                     [ hxy, hyy, hyz ],
                     [ hxz, hyz, hzz]])
	
#
#
def kroon3( input, axis=-1, output=None, mode="reflect", cval=0.0):
	"""Full block first derivative along an axis using a 3 point Kroon filter.
	
	Applies a 3 point Kroon first derivative filter along an axis of the input
	array as per: 
	Kroon, 2009: Numerical optimization of kernel based image derivatives.
	
	Args:
		input: the array to be filtered.
		axis: optional, specifies the array axis to calculate the
			derivative. Default is the last axis.
		output: optional, an array to store the derivative filter output.
			Should be the same shape as the input array.
		mode: {'reflect', 'constant', 'nearest', 'mirror', 'wrap'} optional,
			specifies how the array boundaries are filtered. Default is
			'reflect'.
		cval: optional, specified value to pad input array if mode is 
			'constant'. Default is 0.0.
			
	Returns:
		derivative filtered array with same shape as input. Note for each array
		dimension indices 1:-1 will be free of boundary effects. 
	"""
	input = np.asarray(input)
	axis = ndi._ni_support._check_axis(axis, input.ndim)
	output = getOutput( output, input)
	ndi.correlate1d(input, [-0.5, 0, 0.5], axis, output, mode, cval, 0)
	axes = [ii for ii in range(input.ndim) if ii != axis]
	for ii in axes:
		ndi.correlate1d(output, [0.178947,0.642105,0.178947], ii, output, mode, cval, 0,)
	return output

# Farid 5 point second derivative filter
#
def farid2_( input, axis=-1, output=None, mode="reflect", cval=0.0):
	"""Calculate a size 5 Farid second derivative filter.
	Parameters
	----------
	%(input)s
	%(axis)s
	%(output)s
	%(mode)s
	%(cval)s
	"""
	input = np.asarray(input)
	axis = ndi._ni_support._check_axis(axis, input.ndim)
	output = getOutput(output, input)
	ndi.correlate1d(input, [0.232905, 0.002668, -0.471147, 0.002668, 0.232905], axis, output, mode, cval, 0)
	axes = [ii for ii in range(input.ndim) if ii != axis]
	for ii in axes:
		ndi.correlate1d(output, [0.030320, 0.249724, 0.439911, 0.249724, 0.030320], ii, output, mode, cval, 0,)
	return output

# Farid 5 point derivative filter
#
def farid5( input, axis=-1, output=None, mode="reflect", cval=0.0):
	"""Calculate a size 5 Farid first derivative filter.
	Parameters
	----------
	%(input)s
	%(axis)s
	%(output)s
	%(mode)s
	%(cval)s
	"""
	input = np.asarray(input)
	axis = ndi._ni_support._check_axis(axis, input.ndim)
	output = getOutput(output, input)
	ndi.correlate1d(input, [-0.109604, -0.276691,  0.000000, 0.276691, 0.109604], axis, output, mode, cval, 0)
	axes = [ii for ii in range(input.ndim) if ii != axis]
	for ii in axes:
		ndi.correlate1d(output, [0.037659,  0.249153,  0.426375, 0.249153, 0.037659], ii, output, mode, cval, 0,)
	return output

# Gaussian filter kernel
#
def getGaussian( xs, ys, zs ):
	"""Return a gaussian filter kernel of the specified size
	"""
	tmp = np.zeros((xs,ys,zs))
	tmp[xs//2, ys//2, zs//2] = 1.0
	return ndi.gaussian_filter(tmp, (xs/6,ys/6,zs/6), mode='constant')


# Convolution of 3D filter with 3D data - only calulates the output for the centre trace
# Numba JIT used to accelerate the calculations
#
@jit
def sconvolve(arr, filt):
	X,Y,Z = arr.shape
	Xf,Yf,Zf = filt.shape
	X2 = X//2
	Y2 = Y//2
	Xf2 = Xf//2
	Yf2 = Yf//2
	Zf2 = Zf//2
	result = np.zeros(Z)
	for i in range(Zf2, Z-Zf2):
		num = 0.0
		for ii in range(Xf):
			for jj in range(Yf):
				for kk in range(Zf):
					num += (filt[Xf-1-ii, Yf-1-jj, Zf-1-kk] * arr[X2-Xf2+ii, Y2-Yf2+jj, i-Zf2+kk])
		result[i] = num
	return result

#
#	General vector filtering function
#	indata contains the vector components
#	window is the window length in the Z direction the size in the X and Y directions is determined from the data
#	filtFunc is a Python function that takes an array of vector coordinates and applies the filter
#	outdata is an array that holds the filtered output vectors
def vecFilter(indata, window, filtFunc, outdata ):
    nz = indata.shape[3]
    half_win = window//2
    outdata.fill(0.0) 
    for z in range(half_win,nz-half_win):
        pts = indata[:,:,:,z-half_win:z+half_win+1].reshape(3,-1)
        outdata[:,z] = filtFunc(pts)
    for z in range(half_win):
        outdata[:,z] = outdata[:,half_win]
    for z in range(nz-half_win, nz):
        outdata[:,z] = outdata[:,nz-half_win-1]

#
#	Calculate the mean vector of the contents of the pts array 
def vecmean(pts):
	n = pts.shape[-1]
	dist = np.zeros((n))
	X=Y=Z=0.0
	for i in range(n):
		X += pts[0,i]
		Y += pts[1,i]
		Z += pts[2,i]
	return np.array([X,Y,Z])/n
#
#	Calculate the vector median of the contents of the pts array - this uses absolute distance
def vmf_l1(pts):
    n = pts.shape[-1]
    dist = np.zeros((n))
    for i in range(n):
        for j in range(i+1,n):
            tmp = abs(pts[0,j]-pts[0,i]) + abs(pts[1,j]-pts[1,i]) + abs(pts[2,j]-pts[2,i])
            dist[i] += tmp
            dist[j] += tmp
    return pts[:,np.argmin(dist)]

#
#	Calculate the vector median of the contents of the pts array - this uses squared distance (L2 norm)
def vmf_l2(pts):
    n = pts.shape[-1]
    dist = np.zeros((n))
    for i in range(n):
        for j in range(i+1,n):
            tmp = (pts[0,j]-pts[0,i])**2 + (pts[1,j]-pts[1,i])**2 + (pts[2,j]-pts[2,i])**2
            dist[i] += tmp
            dist[j] += tmp
    return pts[:,np.argmin(dist)]

#
#	Calculate the vector median of the contents of the pts array - this uses Seol and Cheun's X3 L2  norm approximation
def vmf_x3(pts):
    n = pts.shape[-1]
    dist = np.zeros((n))
    for i in range(n):
        for j in range(i+1,n):
            dx = abs(pts[0,j]-pts[0,i])
            dy = abs(pts[1,j]-pts[1,i])
            dz = abs(pts[2,j]-pts[2,i])
            tmp = max(dx,dy,dz)+dx+dy+dz
            dist[i] += tmp
            dist[j] += tmp
    return pts[:,np.argmin(dist)]

#
#	Stride trickery for rolling windows
def rolling_window(a, window):
    shape = a.shape[:-1] + (a.shape[-1] - window + 1, window)
    strides = a.strides + (a.strides[-1],)
    return np.lib.stride_tricks.as_strided(a, shape=shape, strides=strides)
