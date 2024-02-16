# Orientation from Orientation Tensor by Local Polynomial Approximation using Numba JIT Acceleration
#
# Calculates the orientation using an orientation tensor derived from the coefficients of a local polynomial
# approximation (3D 2nd order polynomial using gaussian weighted least squares) as proposed by Farneback.
# 
import sys,os
import numpy as np
#
# Import the module with the I/O scaffolding of the External Attribute
#
sys.path.insert(0, os.path.join(sys.path[0], '..', '..'))
import extattrib as xa
import extlib as xl
#
# These are the attribute parameters
#
xa.params = {
	'Input': 'Input',
	'Output': ['Crl_dip', 'Inl_dip', 'True Dip', 'Dip Azimuth', 'Coherency'],
	'ZSampMargin' : {'Value':[-1,1], 'Symmetric': True},
	'StepOut' : {'Value': [1,1]},
	'Par_0': {'Name': 'Weight Factor', 'Value': 0.2},
	'Select': {'Name': 'Normalzation', 'Values': ['Standard','First','Second','MinMax','Dynamic','Covariance'], 'Selection': 0},
	'Help': xa.HelpRoot
}
#
# Define the compute function
#
def doCompute():
	xs = xa.SI['nrinl']
	ys = xa.SI['nrcrl']
	zs = xa.params['ZSampMargin']['Value'][1] - xa.params['ZSampMargin']['Value'][0] + 1
	wf = xa.params['Par_0']['Value']
	norm = xa.params['Select']['Selection']
	kernel = lpa3D_init(xs, ys, zs, wf)
	gam = 1/(4*((min(xs,ys,zs)-1)*wf)**2)
	inlFactor = xa.SI['zstep']/xa.SI['inldist'] * xa.SI['dipFactor']
	crlFactor = xa.SI['zstep']/xa.SI['crldist'] * xa.SI['dipFactor']
	while True:
		xa.doInput()
		r = np.zeros((10,xa.TI['nrsamp']))
		for i in range(0,10):
			r[i,:] = xl.sconvolve(xa.Input,kernel[i])
#		A = np.rollaxis(np.array([[r[4],r[7]/2, r[8]/2], [r[7]/2, r[5], r[9]/2], [r[8]/2, r[9]/2, r[6]]]),2)
		A = np.rollaxis(np.array([[r[4]*2,r[7], r[8]], [r[7], r[5]*2, r[9]], [r[8], r[9], r[6]*2]]),2)
#		A = np.rollaxis(np.array([[2*r[4]],[2*r[5]],[2*r[6]]]),2)
		AAT = np.einsum('...ij,...jk->...ik', A, np.swapaxes(A,1,2))
		B = np.rollaxis(np.array([[r[1]],[r[2]],[r[3]]]),2)
		BBT = np.einsum('...ij,...jk->...ik', B, np.swapaxes(B,1,2))
		T = np.zeros_like(BBT)
		Gtmp = np.zeros(xa.TI['nrsamp'])
		if (norm == 0):
			T = AAT+gam*BBT
		elif (norm == 1):
			T = BBT
		elif (norm == 2):
			T = AAT
		elif (norm == 3):
#			G = np.sqrt(r[1]*r[1]+r[2]*r[2]+r[3]*r[3])
			G = r[1]*r[1]+r[2]*r[2]+r[3]*r[3]
			Gmin = np.amin(G)
			Gmax = np.amax(G)
			Gtmp = ((G-Gmin)/(Gmax-Gmin))
			G = Gtmp[:,np.newaxis,np.newaxis]
			T = (1-G)*AAT+ G*BBT
		elif (norm == 4):
#			G = np.sqrt(r[1]*r[1]+r[2]*r[2]+r[3]*r[3])
			G = r[1]*r[1]+r[2]*r[2]+r[3]*r[3]
			Groll = xl.rolling_window(G,51)
			Gmin = np.pad(np.amin(Groll,-1),(25,25),'edge')
			Gmax = np.pad(np.amax(Groll,-1),(25,25),'edge')
			Gtmp = ((G-Gmin)/(Gmax-Gmin))
			G = Gtmp[:,np.newaxis,np.newaxis]
			T = (1-G)*AAT + G*BBT
		else:
			Droll = xl.rolling_window(xa.Input, zs)
			T = np.pad(np.cov(Droll),(xa.params['ZSampMargin']['Value'][1],xa.params['ZSampMargin']['Value'][1]),'edge')
		evals, evecs = np.linalg.eigh(T)
		ndx = evals.argsort()
		evecs = evecs[np.arange(0,T.shape[0],1),:,ndx[:,2]]
		eval2 = evals[np.arange(0,T.shape[0],1),ndx[:,2]]
		eval1 = evals[np.arange(0,T.shape[0],1),ndx[:,0]]

		xa.Output['Crl_dip'] = -evecs[:,1]/evecs[:,2]*crlFactor
		xa.Output['Inl_dip'] = -evecs[:,0]/evecs[:,2]*inlFactor
		xa.Output['True Dip'] = np.sqrt(xa.Output['Crl_dip']*xa.Output['Crl_dip']+xa.Output['Inl_dip']*xa.Output['Inl_dip'])
		xa.Output['Dip Azimuth'] = np.degrees(np.arctan2(xa.Output['Inl_dip'],xa.Output['Crl_dip']))
		coh = (eval2-eval1)/(eval2+eval1) 
#		xa.Output['Coherency'] = coh * coh
		xa.Output['Coherency'] = Gtmp

		xa.doOutput()
#
# Find the LPA solution for a 2nd order polynomial in 3D
#
def lpa3D_init( xs, ys, zs, sigma=0.2 ):
	sx = sigma * (xs-1)
	sy = sigma * (ys-1)
	sz = sigma * (zs-1)
	hxs = (xs-1)/2
	hys = (ys-1)/2
	hzs = (zs-1)/2
	xtmp = np.linspace(-hxs,hxs,xs)
	ytmp = np.linspace(-hys,hys,ys)
	ztmp = np.linspace(-hzs,hzs,zs)
	xyz = np.meshgrid(xtmp,ytmp,ztmp, indexing='ij')
	x = xyz[0].flatten()
	y = xyz[1].flatten()
	z = xyz[2].flatten()
	w = np.exp(-(x**2/(2*sx**2) + y**2/(2*sy**2) + z**2/(2*sz**2)))
	W = np.diagflat(w)
	A = np.dstack((np.ones(x.size), x, y, z, x*x, y*y, z*z, x*y, x*z, y*z)).reshape((x.size,10))
	DB = np.linalg.inv(A.T.dot(W).dot(A)).dot(A.T).dot(W)
	return DB.reshape((10,xs,ys,zs))
#
# Assign the compute function to the attribute
#
xa.doCompute = doCompute
#
# Do it
#
xa.run(sys.argv[1:])
  
