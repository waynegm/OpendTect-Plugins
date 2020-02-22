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
	'Inputs': ['Near', 'Mid', 'Far'],
	'Output': ['Intercept', 'Gradient', 'Quality'],
	'Par_0': {'Name': 'Near Angle', 'Value': 6},
	'Par_1': {'Name': 'Mid Angle', 'Value': 17},
	'Par_2': {'Name': 'Far Angle', 'Value': 27},
	'Help': 'http://waynegm.github.io/OpendTect-Plugin-Docs/external_attributes/AVO_IG.html'
}
#
# Define the compute function
#
def doCompute():
	near_ang = xa.params['Par_0']['Value']
	mid_ang = xa.params['Par_1']['Value']
	far_ang = xa.params['Par_2']['Value']
	angs = np.array([near_ang, mid_ang, far_ang])

	while True:
		xa.doInput()

		near = xa.Input['Near'][0,0,:]
		mid = xa.Input['Mid'][0,0,:]
		far = xa.Input['Far'][0,0,:]
		refl = np.array([near,mid,far])
		ns = near.shape[0]
    
		G=np.zeros(ns)
		I=np.zeros(ns)
		Q=np.zeros(ns)
		avo_IG_numpy( refl, angs, I, G, Q)
#
#	Get the output
		xa.Output['Intercept'] = I
		xa.Output['Gradient'] = G
		xa.Output['Quality'] = Q
		xa.doOutput()

def avo_IG_numpy(refls, angles, outI, outG, qual):
	sangs = np.square(np.sin(angles/180*np.pi))
	res = np.linalg.lstsq(np.c_[sangs,np.ones_like(sangs)], refls)
	outI[...] = res[0][1]
	outG[...] = res[0][0]
	qual[...] = 1 - res[1]/(refls.shape[0] * np.var(refls,axis=0))
#
# Assign the compute function to the attribute
#
xa.doCompute = doCompute
#
# Do it
#
xa.run(sys.argv[1:])
