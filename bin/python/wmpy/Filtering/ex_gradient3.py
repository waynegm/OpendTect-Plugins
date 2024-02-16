# Compute Gradients
#
# Prewitt External Attribute
#
import sys,os
import numpy as np
sys.path.insert(0, os.path.join(sys.path[0], '..'))
import extattrib as xa
import extlib as xl

xa.params = {
    'Inputs': ['Input'],
    'Output' : ['Average Gradient', 'In-line gradient', 'Cross-line gradient', 'Z gradient'],
    'ZSampMargin' : {'Value': [-1,1], 'Hidden': True},
    'StepOut' : {'Value': [1,1], 'Hidden': True},
    'Select': {'Name': 'Operator', 'Values': ['Scharr','Kroon'], 'Selection': 0},
    'Parallel' : False,
    'Help'  : xa.HelpRoot
}

def doCompute():
    inlpos = xa.SI['nrinl']//2
    crlpos = xa.SI['nrcrl']//2
    filt = xa.params['Select']['Selection']

    while True:
        xa.doInput()
        indata = xa.Input['Input']
        if filt==0:
            xa.Output['In-line gradient']       = xl.scharr3_dx(indata, full=False)
            xa.Output['Cross-line gradient']    = xl.scharr3_dy(indata, full=False)
            xa.Output['Z gradient']             = xl.scharr3_dz(indata, full=False)
        else:
            xa.Output['In-line gradient']       = xl.kroon3(indata, axis=0)[inlpos,crlpos,:]
            xa.Output['Cross-line gradient']    = xl.kroon3(indata, axis=1)[inlpos,crlpos,:]
            xa.Output['Z gradient']             = xl.kroon3(indata, axis=-1)[inlpos,crlpos,:]

        xa.Output['Average Gradient']   =   (   xa.Output['In-line gradient']
                                            +   xa.Output['Cross-line gradient']
                                            +   xa.Output['Z gradient'] )/3
        xa.doOutput()

xa.doCompute = doCompute
xa.run(sys.argv[1:])


