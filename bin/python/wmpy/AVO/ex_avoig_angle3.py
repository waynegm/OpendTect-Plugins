#
# AVO Intercept and Gradient from 3 angle stacks
#
# Copyright (C) 2016 Wayne Mogg All rights reserved.
#
# This file may be used under the terms of the MIT License
# (https://github.com/waynegm/OpendTect-External-Attributes/blob/master/LICENSE)
#
# Author:		Wayne Mogg
# Date: 		February, 2020
#
import sys,os
import numpy as np
#
# Import the module with the I/O scaffolding of the External Attribute
#
import wmpy.extattrib as xa
#
# These are the attribute parameters
#
xa.params = {
	'Inputs': ['Near Stack', 'Mid Stack', 'Far Stack'],
	'Output': ['Intercept', 'Gradient', 'Quality'],
	'Near Angle (deg)': {'Type': 'Number', 'Value': 6},
	'Mid Angle (deg)': {'Type': 'Number', 'Value': 17},
	'Far Angle (deg)': {'Type': 'Number', 'Value': 27},
	'Help': xa.HelpRoot+'AVO/#avo-intercept-and-gradient'
}
#
# Define the compute function
#
def doCompute():
	near_ang = xa.params['Near Angle (deg)']['Value']
	mid_ang = xa.params['Mid Angle (deg)']['Value']
	far_ang = xa.params['Far Angle (deg)']['Value'] 
	angles = np.radians(np.array([near_ang, mid_ang, far_ang]))

	a = np.ones(angles.size)
	b = np.sin(angles)**2
	coeff = np.array([a, b]).T

	while True:
		xa.doInput()

		near = xa.Input['Near Stack'][0,0,:]
		mid = xa.Input['Mid Stack'][0,0,:]
		far = xa.Input['Far Stack'][0,0,:]
		refls = np.array([near,mid,far])

		res, resid = np.linalg.lstsq(coeff, refls, rcond=None)[:2]
#
#	Get the output
		xa.Output['Intercept'] = res[0]
		xa.Output['Gradient'] = res[1]
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
