# Volumetric curvature from Inline and Crossline dip
# as per Weinkauf and Theisel(2003) "Curvature measures of 3D vector fields and their applications" 
#
import sys,os
import numpy as np
from numba import autojit
#
# Import the module with the I/O scaffolding of the External Attribute
#
sys.path.insert(0, os.path.join(sys.path[0], '..'))
import extattrib as xa
import extlib as xl
#
# These are the attribute parameters
#
xa.params = {
	'Inputs': ['Crl_dip', 'Inl_dip'],
	'Output': ['Mean curvature', 'Gaussian curvature', 'Maximum curvature', 'Minimum curvature', 'Most pos curvature', 'Most neg curvature'],
	'ZSampMargin' : {'Value':[-1,1], 'Hidden': True},
	'StepOut' : {'Value': [1,1], 'Hidden': True},
	'Help': 'http://waynegm.github.io/OpendTect-Plugin-Docs/Attributes/ExternalAttrib/'
}
#
# Define the compute function
#
def doCompute():
	hxs = xa.SI['nrinl']//2
	hys = xa.SI['nrcrl']//2
	inlFactor = xa.SI['zstep']/xa.SI['inldist'] * xa.SI['dipFactor']
	crlFactor = xa.SI['zstep']/xa.SI['crldist'] * xa.SI['dipFactor']
	while True:
		xa.doInput()

		U = -xa.Input['Inl_dip']/inlFactor
		V = -xa.Input['Crl_dip']/crlFactor
		W = np.ones(U.shape)
#
#	Calculate the derivatives of the components and some intermediate results
		ux = xl.kroon3( U, axis=0 )[hxs,hys,:]
		uy = xl.kroon3( U, axis=1 )[hxs,hys,:]
		uz = xl.kroon3( U, axis=2 )[hxs,hys,:]
		vx = xl.kroon3( V, axis=0 )[hxs,hys,:]
		vy = xl.kroon3( V, axis=1 )[hxs,hys,:]
		vz = xl.kroon3( V, axis=2 )[hxs,hys,:]
		wx = xl.kroon3( W, axis=0 )[hxs,hys,:]
		wy = xl.kroon3( W, axis=1 )[hxs,hys,:]
		wz = xl.kroon3( W, axis=2 )[hxs,hys,:]
		u = U[hxs,hys,:]
		v = V[hxs,hys,:]
		w = W[hxs,hys,:]
		uv = u*v
		vw = v*w
		u2 = u*u
		v2 = v*v
		w2 = w*w
		u2pv2 = u2+v2
		v2pw2 = v2+w2
		s = np.sqrt(u2pv2+w2)
#
#	Compute the basic measures of Gauss's theory of surfaces
		E = np.ones(u.shape)
		F = -(u*w)/(np.sqrt(u2pv2)*np.sqrt(v2pw2))
		G = np.ones(u.shape)
		D = -(-uv*vx+u2*vy+v2*ux-uv*uy)/(u2pv2*s)
		Di = -(vw*(uy+vx)-2*u*w*vy-v2*(uz+wx)+uv*(vz+wy))/(2*np.sqrt(u2pv2)*np.sqrt(v2pw2)*s)
		Dii = -(-vw*wy+v2*wz+w2*vy-vw*vz)/(v2pw2*s)
		H = (E*Dii - 2*F*Di + G*D)/(2*(E*G - F*F))
		K = (D*Dii - Di*Di)/(E*G - F*F)
		Kmin = H - np.sqrt(H*H - K)
		Kmax = H + np.sqrt(H*H - K)
#
#	Get the output
		xa.Output['Mean curvature'] = H
		xa.Output['Gaussian curvature'] = K
		xa.Output['Maximum curvature'] = Kmax
		xa.Output['Minimum curvature'] = Kmin
		xa.Output['Most pos curvature'] = np.maximum(Kmax,Kmin)
		xa.Output['Most neg curvature'] = np.minimum(Kmax,Kmin)
		xa.doOutput()
#
# Assign the compute function to the attribute
#
xa.doCompute = doCompute
#
# Do it
#
xa.run(sys.argv[1:])
  
