# Post-stack inversion using the pylops.avo.poststack.PoststackInversion operator
#
# Input: Seismic data to be inverted
# Output: Relative acoustic impedance or inversion residual
#
# User must ensure:
#   - SURVEY parameter on line 46 provides the path to the OpendTect survey/project
#   - WAVELET parameter on line 47 contains the filename of the OpendTect wavelet to use
#
import sys,os
import numpy as np
import pylops
#
# Import the module with the I/O scaffolding of the External Attribute
#
sys.path.insert(0, os.path.join(sys.path[0], '..'))
import extattrib as xa

#
# The attribute parameters - keep what you need
#
xa.params = {
  'Inputs' : ['Seismic'],
  'Output' : ['Impedance', 'Residual'],
  'Wavelet' : {'Type': 'File', 'Value': 'Seismics/*.wvlt'},
  'Regularization (%)'  : {'Type': 'Number', 'Value': 0.1},
  'Mode' : {'Type': 'Select', 'Options': ['normal', 'blocky'], 'Value': 'normal'},
  'Parallel' : False,
  'Help'  : 'http://waynegm.github.io/OpendTect-Plugin-Docs/Attributes/ExternalAttrib/'
}
#
# Define the compute function
#
def doCompute():
#
# Set some constants based on the users parameter input
#
  dt_sec = xa.SI['zstep']
  mode = xa.params['Mode']['Value']
  waveletfile = xa.params['Wavelet']['Value']
  reg = xa.params['Regularization (%)']['Value']/100
#
# Load the wavelet
#
  wavelet = np.loadtxt(waveletfile, comments='!', skiprows=9)
#
# Do it
#
  nrinput = xa.SI['nrinput']
  epsI = None
  while True:
    xa.doInput()
    nrsamp = xa.TI['nrsamp']
#
#   Collect the input data
#
    indata = xa.Input['Seismic'][0,0,:]
    if not epsI:
      epsI = reg * np.max(indata)
#
#   Initialise arrays for the results
#
    minv = np.zeros((nrsamp))
    residual = np.zeros((nrsamp))
#
#   Invert
#
    if mode=='normal':
        minv, residual = pylops.avo.poststack.PoststackInversion(indata, wavelet, m0=np.zeros_like(indata),
                                          explicit=True, simultaneous=False, epsI=epsI)
    elif mode=='blocky':
        epsI_b = epsI # damping
        epsRL1_b = epsI # blocky regularization
        minv, residual = pylops.avo.poststack.PoststackInversion(indata, wavelet, m0=np.zeros_like(indata),
                         explicit=False, epsRL1=[epsRL1_b],
                         **dict( damp=epsI_b))

#
#   Save the results
#
    xa.Output['Impedance'] = minv
    xa.Output['Residual'] = residual

    xa.doOutput()

#
# Assign the compute function to the attribute
#
xa.doCompute = doCompute
#
# Do it
#
xa.run(sys.argv[1:])
  

