# Create a post-stack seismic volume from a logAI model using the pylops.avo.poststack.PoststackLinearModelling operator
#
# Input: Volume with logAI (Acoustic Impedance) data to be modelled
# Output: The reflectivity model filtered by the selected wavelet
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
# The attribute parameters
#
xa.params = {
  'Inputs': ['Acoustic Impedance'],
  'Wavelet' : {'Type': 'File', 'Value': 'Seismics/*.wvlt'},
  'Parallel' : False,
  'Help'  : 'http://waynegm.github.io/WMPlugin-Docs/docs/externalattributes/pylops/'
}
#
# Define the compute function
#
def doCompute():
#
# Set some constants based on the users parameter input
#
  waveletfile = xa.params['Wavelet']['Value']
#
# Load the wavelet
#
  wavelet = np.loadtxt(waveletfile, comments='!', skiprows=9)
#
# Do it
#
  while True:
    xa.doInput()
    nrsamp = xa.TI['nrsamp']
#
#   Find first and last non-zero data in each input
#
    logAI = xa.Input['Acoustic Impedance'][0,0,:]
    nzAI = np.nonzero(logAI)[0]
#
#   Initialise an array for the results
#
    result = np.zeros_like(logAI)
#
#     Replace any leading and trailing 0 values in the input
#
    logAI[0:nzAI[0]] = logAI[nzAI[0]]
    logAI[nzAI[-1]:] = logAI[nzAI[-1]]
#
#     Compute the linear operator for the model
#
    PPop = pylops.avo.poststack.PoststackLinearModelling(wavelet/2, nt0=nrsamp)
#
#     Compute the model output and make it ready for output
#
    result = PPop * logAI
    result = result.reshape(-1)
    result[0:nzAI[0]] = xa.undef
    result[nzAI[-1]:] = xa.undef
#
#   Save the results
#
    xa.Output = result

    xa.doOutput()

#
# Assign the compute function to the attribute
#
xa.doCompute = doCompute
#
# Do it
#
xa.run(sys.argv[1:])


