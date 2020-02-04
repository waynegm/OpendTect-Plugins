# Rectangular Spatial Filter
#
# Applies a Lowpass, Highpass, Band Reject or Bandpass rectangular spatial (k-k) filter
# by convolution 
# Note setting a stepout of 0 will apply the filter in a single direction 
#
import sys,os
import numpy as np
import scipy.special as ss
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
	'StepOut' : {'Value': [9,9], 'Hidden': False},
	'Par_0' : {'Name': 'Norm. Inline Spatial Freq', 'Value': 0.5},
	'Par_1' : {'Name': 'Norm. Xline Spatial Freq', 'Value': 0.5},
	'Select' : {'Name': 'Type', 'Values': ['Low Pass', 'High Pass', 'Band Pass', 'Band Reject'], 'Selection': 0},
	'Help'  : 'http://waynegm.github.io/OpendTect-Plugin-Docs/external_attributes/Spatial_Filter_Rectangular.html'
}
#
# Define the compute function
#
def doCompute():
#
#	Compute the filter kernel
#
	nil = xa.SI['nrinl']
	nxl = xa.SI['nrcrl']
	centre_trace_x = nil//2
	centre_trace_y = nxl//2
	ifreq = xa.params['Par_0']['Value']
	xfreq = xa.params['Par_1']['Value']
	type = xa.params['Select']['Selection']
	kernelFunc = lpKernel if type==0 else hpKernel if type==1 else bpKernel if type==2 else brKernel
	ikernel = np.ones((nil,1))
	xkernel = np.ones((nxl,1))
	Ni = nil//2
	Nx = nxl//2
	if (nil != 1):
		if (Ni%2 == 0):
			ikernel[1:2*Ni+2] = kernelFunc(Ni, ifreq)
		else:
			ikernel = kernelFunc(Ni, ifreq)
	if (nxl != 1):
		if (Nx%2 == 0):
			xkernel[1:2*Nx+2] = kernelFunc(Nx,xfreq)
		else:
			xkernel = kernelFunc(Nx,xfreq)
	kernel = np.dot(ikernel, xkernel.T).reshape(nil,nxl,1)
#
#	This is the trace processing loop
#
	while True:
		xa.doInput()
#
#	Get the input
#
		indata = xa.Input['Input']
#
#	Apply the kernel
#
		outdata = np.sum(kernel * indata, axis=(0,1))
#------------------------------------------------------------------------------------
#
		xa.Output = outdata
		xa.doOutput()

def lpKernel(N, freq):
#
# Lowpass filter 1D MAXFLAT kernel generator
#
# N is the filter half-size, must be odd
# freq is the normalised cutoff frequency
#
# Returns the filter kernel of size (2N+1)
#
	num = 2*N + 1
	result = np.zeros((num,1))
	for n in range(N+1):
		i = n+N
		im = N-n
		if (n==0):
			result[i] = freq
		else:
			p = n%2
			val = (ss.factorial2(N)**2 * np.pi**(p-1) * np.sin(n*np.pi*freq)) / ( 2**p * n * ss.factorial2(i) * ss.factorial2(im) )
			result[i] = val
			result[im] = val
	return result/np.sum(result)

def hpKernel(N, freq):
#
# Highpass 1D MAXFLAT filter kernel generator
#
# N is the filter half-size, must be odd
# freq is the normalised cutoff frequency
#
# Returns the filter kernel of size (2N+1)
#
	result = lpKernel(N, freq)
	for n in range(-N,N+1):
		i = n+N
		if (n==0):
			result[i] = 1-result[i]
		else:
			result[i] = -result[i]
	return result

def brKernel(N, freq, halfwidth=0.1):
#
# Band Reject 1D MAXFLAT filter kernel generator
#
# N is the filter half-size, must be odd
# freq is the normalised centre frequency of the reject band
# halfwidth controls the aperture of the reject band to freq +/- halfwidth
#
# Returns the filter kernel of size (2N+1)
#
	kernel_lp = lpKernel(N, freq-halfwidth)
	kernel_hp = hpKernel(N, freq+halfwidth)
	result = kernel_lp + kernel_hp
	return result

def bpKernel(N, freq , halfwidth=0.1):
#
# Bandpass 1D MAXFLAT filter kernel generator
#
# N is the filter half-size, must be odd
# freq is the normalised centre frequency of the pass band
# halfwidth controls the aperture of the pass band to freq +/- halfwidth
#
# Returns the filter kernel of size (2N+1)
#
	result = brKernel(N, freq, halfwidth)
	for n in range(-N,N+1):
		i = n+N
		if (n==0):
			result[i] = 1-result[i]
		else:
			result[i] = -result[i]
	return result
#
# Assign the compute function to the attribute
#
xa.doCompute = doCompute
#
# Do it
#
xa.run(sys.argv[1:])
  

