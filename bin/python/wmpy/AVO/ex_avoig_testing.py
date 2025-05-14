#
# Generate AVO Intercept and Gradient test data
#
# Copyright (C) 2025 Wayne Mogg All rights reserved.
#
# This file may be used under the terms of the MIT License
# (https://github.com/waynegm/OpendTect-External-Attributes/blob/master/LICENSE)
#
# Author:		Wayne Mogg
# Date: 		May, 2025
#
import sys,os
import numpy as np
#
# Import the module with the I/O scaffolding of the External Attribute
#
import wmpy.extattrib as xa
from wmpy.extlib import makeAVOClassLogs, shueyIG
#
# These are the attribute parameters
#
xa.params = {
	'Inputs': ['Input'],
	'Output': ['Intercept', 'Gradient'],
	'Include' : {'Type': 'Select', 'Options':['Gas Sand Only','Brine Sand Only','Both'], 'Value':'Both'},
	'Parallel': False,
	'Help': xa.HelpRoot+'AVO/#avo-intercept-and-gradient'
}

#
# Define the compute function
#
def doCompute():
	xa.logH.setLevel(xa.logging.INFO)
	sands = xa.params['Include']['Value']
	withgas = True if sands in ['Gas Sand Only', 'Both'] else False
	withbrine = True if sands in ['Brine Sand Only', 'Both'] else False
	nrz = xa.SI['nrZ']
	vp, vs, rhob = makeAVOClassLogs(nrz, withgas, withbrine)

	while True:
		xa.doInput()
		nsamp = xa.TI['nrsamp']
		z0 = xa.TI['z0']
		intercept, gradient = shueyIG(vp, vs, rhob)
		if z0+nsamp>len(intercept):
			intercept.resize(z0+nsamp)
			gradient.resize(z0+nsamp)
		
#	Get the output
		xa.Output['Intercept'] = intercept[z0:z0+nsamp]
		xa.Output['Gradient'] = gradient[z0:z0+nsamp]
		xa.doOutput()
#
# Assign the compute function to the attribute
#
xa.doCompute = doCompute
#
# Do it
#
xa.run(sys.argv[1:])
