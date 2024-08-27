#
# 3 term fatti reflectivity from 4 angle stacks
#
# Copyright (C) 2024 Wayne Mogg All rights reserved.
#
# This file may be used under the terms of the MIT License
#
# Author:               Wayne Mogg
# Date:                 March, 2024
#
import sys,os
import numpy as np
import math
#
# Import the module with the I/O scaffolding of the External Attribute
#
import wmpy.extattrib as xa
#
# These are the attribute parameters
#
xa.params = {
    'Inputs': ['Near Stack', 'Mid Stack', 'Far Stack', 'UltraFar Stack'],
    'Output': ['Rp', 'Rs', 'Rd', 'Quality'],
    'Near Angle (deg)': {'Type': 'Number', 'Value': 5},
    'Mid Angle (deg)': {'Type': 'Number', 'Value': 15},
    'Far Angle (deg)': {'Type': 'Number', 'Value': 25},
    'UltraFar Angle (deg)': {'Type': 'Number', 'Value': 35},
    'Vp/Vs' : {'Type': 'Number', 'Value': 1.5},
    'Parallel' : True,
    'Help' : xa.HelpRoot+'AVO/#avo-fatti-3-term'
}
#
# Define the compute function
#
def doCompute():

    near_ang = xa.params['Near Angle (deg)']['Value']
    mid_ang = xa.params['Mid Angle (deg)']['Value']
    far_ang = xa.params['Far Angle (deg)']['Value'] 
    ufar_ang = xa.params['UltraFar Angle (deg)']['Value'] 
    vpvs = xa.params['Vp/Vs']['Value']

    angles = np.radians(np.array([near_ang, mid_ang, far_ang, ufar_ang]))
    a = (1+np.tan(angles)**2)
    b = -8*np.sin(angles)**2/vpvs**2
    c = -(np.tan(angles)**2-4*np.sin(angles)**2/vpvs**2)
    coeff = np.array([a, b, c]).T    

    while True:
        xa.doInput()

        near = xa.Input['Near Stack'][0,0,:]
        mid = xa.Input['Mid Stack'][0,0,:]
        far = xa.Input['Far Stack'][0,0,:]
        ufar = xa.Input['UltraFar Stack'][0,0,:]
        refls = np.array([near,mid,far,ufar])

        res, resid = np.linalg.lstsq(coeff, refls, rcond=None)[:2]
#
#       Get the output
        xa.Output['Rp'] = res[0]
        xa.Output['Rs'] = res[1]
        xa.Output['Rd'] = res[2]
        xa.Output['Quality'] = 1 - resid/(refls.size * refls.var())
        xa.doOutput()
#
# Assign the compute function to the attribute
#
xa.doCompute = doCompute
#
# Do it
#
xa.run(sys.argv[1:])
