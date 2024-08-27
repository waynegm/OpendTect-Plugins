#
# AVO Intercept and Gradient from 5 angle stacks
#
# Copyright (C) 2016 Wayne Mogg All rights reserved.
#
# This file may be used under the terms of the MIT License
# (https://github.com/waynegm/OpendTect-External-Attributes/blob/master/LICENSE)
#
# Author:		Wayne Mogg
# Date: 		February, 2020
# Homepage:		http://waynegm.github.io/OpendTect-Plugin-Docs/External_Attributes/ExternalAttributes/
#
import sys,os
import numpy as np
#
# Import the module with the I/O scaffolding of the External Attribute
#
sys.path.insert(0, os.path.join(sys.path[0], '..'))
import extattrib as xa
#
# These are the attribute parameters
#
xa.params = {
	'Inputs': ['Near Stack', 'Mid Stack', 'Far Stack', 'ExtraFar Stack', 'UltraFar Stack'],
	'Output': ['Intercept', 'Gradient', 'Quality'],
	'Near Angle (deg)': {'Type': 'Number', 'Value': 5},
	'Mid Angle (deg)': {'Type': 'Number', 'Value': 15},
	'Far Angle (deg)': {'Type': 'Number', 'Value': 25},
	'ExtraFar Angle (deg)': {'Type': 'Number', 'Value': 35},
	'UltraFar Angle (deg)': {'Type': 'Number', 'Value': 45},
	'Help': xa.HelpRoot+'AVO/#avo-intercept-and-gradient'
}
#
# Define the compute function
#
def doCompute():
	near_ang = xa.params['Near Angle (deg)']['Value']
	mid_ang = xa.params['Mid Angle (deg)']['Value']
	far_ang = xa.params['Far Angle (deg)']['Value'] 
	xfar_ang = xa.params['ExtraFar Angle (deg)']['Value'] 
	ufar_ang = xa.params['UltraFar Angle (deg)']['Value'] 
	angles = np.radians(np.array([near_ang, mid_ang, far_ang, xfar_ang, ufar_ang]))

	a = np.ones(len(angles))
	b = np.sin(angles)**2
	coeff = np.array([a, b]).T

	while True:
		xa.doInput()

		near = xa.Input['Near Stack'][0,0,:]
		mid = xa.Input['Mid Stack'][0,0,:]
		far = xa.Input['Far Stack'][0,0,:]
		xfar = xa.Input['ExtraFar Stack'][0,0,:]
		ufar = xa.Input['UltraFar Stack'][0,0,:]
		refls = np.array([near,mid,far,xfar,ufar])

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
