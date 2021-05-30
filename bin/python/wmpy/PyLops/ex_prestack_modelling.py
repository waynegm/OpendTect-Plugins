# Create a pre-stack angle volume using the pylops.avo.avo.AVOLinearModelling operator
#
# Input: Log cubes with the compressional sonic (DT in us/m), shear sonic (DTS in us/m) 
#        and density (RHOB in g/cc) data to be modelled
# Output: The reflectivity model at the specified angle filtered by the selected wavelet
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
  'Inputs': ['DT(us/m)', 'DTS(us/m)', 'RHOB(g/cc)'],
  'Wavelet' : {'Type': 'File', 'Value': 'Seismics/*.wvlt'},
  'Angle (deg)' : {'Type': 'Number', 'Value': 6},
  'Method' : {'Type': 'Select', 'Options': ['akirich', 'fatti'], 'Value': 'akirich'},
  'Parallel' : False,
  'Help'  : 'http://waynegm.github.io/OpendTect-Plugin-Docs/Attributes/ExternalAttrib/PyLops.html'
}
#
# Define the compute function
#
def doCompute():
#
# Set some constants based on the users parameter input
#
  dt_sec = xa.SI['zstep']
  angle = xa.params['Angle (deg)']['Value']
  method = xa.params['Method']['Value']
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
    DT = np.array(xa.Input['DT(us/m)'].reshape(-1))
    nzdt = np.nonzero(DT)[0]
    DTS = np.array(xa.Input['DTS(us/m)'].reshape(-1))
    nzdts = np.nonzero(DTS)[0]
    rhob = np.array(xa.Input['RHOB(g/cc)'].reshape(-1))
    nzrhob = np.nonzero(rhob)[0]
#
#   Initialise an array for the results
#
    result = np.zeros_like(DT)
#
#   Only compute the model if all 3 inputs have data
#
    if len(nzdt)>0 and len(nzdts)>0 and len(nzrhob)>0:
#
#     For each input replace any leading and trailing 0 values before computing the modeling input 
#
      DT[0:nzdt[0]] = DT[nzdt[0]]
      DT[nzdt[-1]:] = DT[nzdt[-1]]
      vp = 1000000/DT

      DTS[0:nzdts[0]] = DTS[nzdts[0]]
      DTS[nzdts[-1]:] = DTS[nzdts[-1]]
      vs = 1000000/DTS

      rhob[0:nzrhob[0]] = rhob[nzrhob[0]]
      rhob[nzrhob[-1]:] = rhob[nzrhob[-1]]
#
#     Compute the linear operator for the model
#
      model = np.stack((np.log(vp), np.log(vs), np.log(rhob)), axis=1)
      vsvpmean = np.average(vs/vp)
      PPop_const = pylops.avo.prestack.PrestackLinearModelling(wavelet, np.array([angle]), vsvp=vsvpmean, nt0=nrsamp,
                                                linearization=method)
#
#     Compute the model output and make it ready for output
#
      result = PPop_const *model.flatten()
      result = result.reshape(-1)
      result[0:nzdt[0]] = xa.undef
      result[nzdt[-1]:] = xa.undef
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
  

