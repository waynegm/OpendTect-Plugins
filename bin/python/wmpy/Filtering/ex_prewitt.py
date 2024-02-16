# Prewitt External Attribute
import sys,os
import numpy as np
from scipy.ndimage import prewitt
sys.path.insert(0, os.path.join(sys.path[0], '..'))
import extattrib as xa

xa.params = {
	'Inputs': ['Input'],
	'Output' : ['Average Gradient', 'In-line gradient', 'Cross-line gradient', 'Z gradient'],
	'ZSampMargin' : {'Value': [-1,1], 'Hidden': True},
	'StepOut' : {'Value': [1,1], 'Hidden': True},
	'Parallel' : False,
	'Help'  : xa.HelpRoot
}

def doCompute():
	inlpos = xa.SI['nrinl']//2
	crlpos = xa.SI['nrcrl']//2

	while True:
		xa.doInput()
		indata = xa.Input['Input']
		xa.Output['In-line gradient'] 		= prewitt(indata, axis=0)[inlpos,crlpos,:]
		xa.Output['Cross-line gradient'] 	= prewitt(indata, axis=1)[inlpos,crlpos,:]
		xa.Output['Z gradient'] 			= prewitt(indata, axis=2)[inlpos,crlpos,:]
		xa.Output['Average Gradient']		= 	(	xa.Output['In-line gradient']
												+	xa.Output['Cross-line gradient']
												+ xa.Output['Z gradient'] )/3
		xa.doOutput()

xa.doCompute = doCompute
xa.run(sys.argv[1:])


