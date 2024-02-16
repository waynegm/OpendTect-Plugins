# Post-stack inversion using the pylops.avo.poststack.PoststackInversion operator
#
# Input: Seismic data to be inverted
#        Background logAI (Acoustic Impedance) model
# Output: Acoustic Impedance or inversion residual
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
  'Inputs' : ['Seismic','Background Model'],
  'Output' : ['Impedance', 'Residual'],
  'Wavelet' : {'Type': 'File', 'Value': 'Seismics/*.wvlt'},
  'Regularization (%)'  : {'Type': 'Number', 'Value': 1},
  'Parallel' : False,
  'Help'  : xa.HelpRoot+'pylops/'
}
#
# Define the compute function
#
def doCompute():
#
# Set some constants based on the users parameter input
#
  dt_sec = xa.SI['zstep']
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
    backgr = xa.Input['Background Model'][0,0,:]
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
    minv, residual = pylops.avo.poststack.PoststackInversion(indata, wavelet/2, m0=backgr,
                                          explicit=True, simultaneous=False, epsI=epsI)
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


