# Rectangular Spatial Filter
#
# Applies a Lowpass, Highpass, Band Reject or Bandpass rectangular spatial (k-k) filter
# by convolution
# Note setting a stepout of 0 will apply the filter in a single direction
#
import sys,os
import numpy as np
#
# Import the module with the I/O scaffolding of the External Attribute
#
sys.path.insert(0, os.path.join(sys.path[0], '..'))
import extattrib as xa
import extlib as xl
#
# The attribute parameters
#
xa.params = {
	'Inputs': ['Input'],
	'StepOut' : {'Value': [9,9], 'Hidden': False},
	'Par_0' : {'Name': 'Norm. Inline Spatial Freq', 'Value': 0.5},
	'Par_1' : {'Name': 'Norm. Xline Spatial Freq', 'Value': 0.5},
	'Select' : {'Name': 'Type', 'Values': ['Low Pass', 'High Pass', 'Band Pass', 'Band Reject'], 'Selection': 0},
	'Help'  : xa.HelpRoot'spatialfilterrectangular/'
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
	kernelFunc = xl.lowpass_kernel if type==0 else xl.highpass_kernel if type==1 else xl.bandpass_kernel if type==2 else xl.nandreject_kernel
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
	ikernel = ikernel.reshape(nil,1)
	xkernel = xkernel.reshape(1,nxl)
	kernel = np.dot(ikernel, xkernel).reshape(nil,nxl,1)
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
#
# Assign the compute function to the attribute
#
xa.doCompute = doCompute
#
# Do it
#
xa.run(sys.argv[1:])


