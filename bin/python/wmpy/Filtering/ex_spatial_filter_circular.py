# Spatial Filter
#
# Applies a Lowpass, Highpass, Band Reject or Bandpass circular symmetric filter
# by convolution 
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
	'StepOut' : {'Value': [9,9], 'Minimum': [9,9], 'Hidden': False, 'Same': True},
	'Par_0' : {'Name': 'Norm. Spatial Frequency', 'Value': 0.5},
	'Select' : {'Name': 'Type', 'Values': ['Low Pass', 'High Pass', 'Band Pass', 'Band Reject'], 'Selection': 0},
	'Help'  : 'http://waynegm.github.io/OpendTect-Plugin-Docs/external_attributes/Spatial_Filter_Circular.html'
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
	freq = xa.params['Par_0']['Value']
	type = xa.params['Select']['Selection']
	kernelFunc = lpKernel if type==0 else hpKernel if type==1 else bpKernel if type==2 else brKernel
	kernel = np.zeros((nil,nil,1))
	N = nil//2
	if (N%2 == 0):
		N=N-1
		kernel[1:2*N+2,1:2*N+2,1] = kernelFunc(N, freq)
	else:
		kernel = kernelFunc(N, freq)
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
# Lowpass filter kernel generator
#
# N is the filter half-size, must be odd
# freq is the normalised cutoff frequency
#
# Returns the filter kernel of size (2N+1,2N+1)
#
    num = 2*N + 1
    result = np.zeros((num,num,1))
    for m in range(N+1):
        i = m+N
        im = -m+N
        for n in range(N+1):
            j = n+N
            jm = -n+N
            if (m==0 and n==0):
                result[i,j] = np.pi*freq**2/4
            else:
                pandq = m%2 + n%2
                mandn = np.sqrt(m*m+n*n)
                val = (ss.factorial2(N)**4 * np.pi**pandq * freq * ss.jn(1,np.pi * freq * mandn)) / ( 2**(pandq+1) * ss.factorial2(N+m) * ss.factorial2(N-m) * ss.factorial2(N+n) *ss.factorial2(N-n) *mandn )
                result[i,j] = val
                result[j,i] = val
                result[im,j] = val
                result[i,jm] = val
                result[im,jm] = val
                
    return result/np.sum(result)

def hpKernel(N, freq):
#
# Highpass circular symmetric filter kernel generator
#
# N is the filter half-size, must be odd
# freq is the normalised cutoff frequency
#
# Returns the filter kernel of size (2N+1,2N+1)
#
    result = lpKernel(N, freq)
    for m in range(-N,N+1):
        i = m+N
        for n in range(-N,N+1):
            j = n+N
            if (m==0 and n==0):
                result[i,j] = 1-result[i,j]
            else:
                result[i,j] = -result[i,j]
    return result

def brKernel(N, freq, halfwidth=0.1):
#
# Band Reject circular symmetric filter kernel generator
#
# N is the filter half-size, must be odd
# freq is the normalised centre frequency of the reject band
# halfwidth controls the aperture of the reject band to freq +/- halfwidth
#
# Returns the filter kernel of size (2N+1,2N+1)
#
    kernel_lp = lpKernel(N, freq-halfwidth)
    kernel_hp = hpKernel(N, freq+halfwidth)
    result = kernel_lp + kernel_hp
    return result

def bpKernel(N, freq , halfwidth=0.1):
#
# Bandpass circular symmetric filter kernel generator
#
# N is the filter half-size, must be odd
# freq is the normalised centre frequency of the pass band
# halfwidth controls the aperture of the pass band to freq +/- halfwidth
#
# Returns the filter kernel of size (2N+1,2N+1)
#
    result = brKernel(N, freq, halfwidth)
    for m in range(-N,N+1):
        i = m+N
        for n in range(-N,N+1):
            j = n+N
            if (m==0 and n==0):
                result[i,j] = 1-result[i,j]
            else:
                result[i,j] = -result[i,j]
    return result
#
# Assign the compute function to the attribute
#
xa.doCompute = doCompute
#
# Do it
#
xa.run(sys.argv[1:])
  

