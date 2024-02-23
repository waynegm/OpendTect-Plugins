# External Attribute New UI Tests
#
import sys,os
import numpy as np
#
# Import the module with the I/O scaffolding of the External Attribute
#
sys.path.insert(0, os.path.join(sys.path[0], '..'))
import extattrib as xa

#
# The attribute parameters - keep what you need
#
xa.params = {
	'Inputs': ['Input'],
	'Output': ['Out'],
	'ZSampMargin' : {'Value': [-30,30], 'Minimum': [-1,1], 'Symmetric': True, 'Hidden': False},
	'StepOut' : {'Value': [1,1], 'Minimum': [1,1], 'Hidden': False},
	'File Input' : {'Type': 'File', 'Mode':'FileIn', 'Value': 'Seismics/*.wvlt'},
	'File Output' : {'Type': 'File', 'Mode':'FileOut', 'Value': 'Seismics/*.wvlt'},
	'Dir' : {'Type': 'File', 'Mode':'Dir', 'Value': '*'},
	'Number Input'  : {'Type': 'Number', 'Value': 1},
	'String Input' : {'Type': 'Text', 'Value': 'Bla bla bla'},
	'Another Number Input'  : {'Type': 'Number', 'Value': 10},
	'New Selection' : {'Type': 'Select', 'Options':['New First','New Second','New Third'], 'Value':'New Second'},
	'Another String Input' : {'Type': 'Text', 'Value': 'Some stuff'},
	'Parallel' : False,
	'Help'  : xa.HelpRoot
}
#
# Define the compute function
#
def doCompute():
#
	xa.logH.setLevel(xa.logging.INFO)
	xa.logH.info('Params: %s', xa.params)
#
	while True:
		xa.doInput()
#
		inp = xa.Input['Input']
#
		xa.Output['Out'] = inp[0,0,:]
		xa.doOutput()

#
xa.doCompute = doCompute
#
xa.run(sys.argv[1:])
  

