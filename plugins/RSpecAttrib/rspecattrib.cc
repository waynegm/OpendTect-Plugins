/*
*   RSpecAttrib Plugin
*   Copyright (C) 2019  Wayne Mogg
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program. If not, see <http://www.gnu.org/licenses/>.
*/


#include "rspecattrib.h"
#include <complex>
#include "eigenfilt.h"

#include "attribdataholder.h"
#include "attribdesc.h"
#include "attribfactory.h"
#include "attribparam.h"
#include "survinfo.h"

#include "commondefs.h"

#include "mymath.h"


namespace Attrib

{
mAttrDefCreateInstance(RSpecAttrib)    
    
void RSpecAttrib::initClass()
{
	mAttrStartInitClassWithDescAndDefaultsUpdate

    ZGateParam* gate = new ZGateParam( gateStr() );
    gate->setLimits( Interval<float>(-mLargestZGate,mLargestZGate) );
    gate->setDefaultValue( Interval<float>(-20, 20) );
    desc->addParam( gate );

	FloatParam* step = new FloatParam( stepStr() );
	step->setRequired( false );
	step->setDefaultValue( 5.0f);
	desc->addParam( step );
    
    BoolParam* reassign_ = new BoolParam(reassignStr());
    reassign_->setDefaultValue(false);
    desc->addParam(reassign_);
	
	desc->addOutputDataType( Seis::UnknowData );
	desc->addInput( InputSpec("Input Volume",true) );

	desc->setLocality( Desc::SingleTrace );
	mAttrEndInitClass
}

void RSpecAttrib::updateDesc( Desc& desc )
{
	float dfreq;
	mGetFloatFromDesc( desc, dfreq, stepStr() );
	const float nyqfreq = 0.5f / SI().zStep();
	const int nrattribs = (int)( nyqfreq / dfreq );
	desc.setNrOutputs( Seis::UnknowData, nrattribs );
}

void RSpecAttrib::updateDefaults( Desc& desc )
{
	ValParam* paramgate = desc.getValParam(gateStr());
	mDynamicCastGet( ZGateParam*, zgate, paramgate )
	float roundedzstep = SI().zStep()*SI().zDomain().userFactor();
	if ( roundedzstep > 0 )
		roundedzstep = Math::Floor( roundedzstep );
    zgate->setDefaultValue( Interval<float>(-roundedzstep*5, roundedzstep*5) );
}

RSpecAttrib::RSpecAttrib( Desc& desc )
    : Provider( desc )
{
    if ( !isOK() ) return;

    mGetFloat( step_, stepStr() );
    mGetFloatInterval( gate_, gateStr() );
    mGetBool(reassign_, reassignStr());
	gate_.scale(1.0f/zFactor());
	window_ = gate_.stop - gate_.start;
	float refstep = getRefStep();
//    zsampMargin_ = Interval<int>(mNINT32((gate_.start-window_/2.0)/refstep)-1, mNINT32((gate_.stop+window_/2.0)/refstep)+1);
    zsampMargin_ = Interval<int>(mNINT32(((gate_.start+gate_.stop)/2.0-2.0*window_)/refstep)-1, mNINT32(((gate_.start+gate_.stop)/2.0+2.0*window_)/refstep)+1);
}

bool RSpecAttrib::getInputData( const BinID& relpos, int zintv )
{
	indata_ = inputs_[0]->getData( relpos, zintv );
	if ( !indata_ )
		return false;

	indataidx_ = getDataIndex( 0 );
	
    return true;
}

bool RSpecAttrib::areAllOutputsEnabled() const
{
	for (int idx=0; idx<nrOutputs(); idx++)
		if (!outputinterest_[idx])
			return false;
	return true;
}

void RSpecAttrib::getCompNames( BufferStringSet& nms ) const
{
	nms.erase();
	const float fnyq = 0.5f / refstep_;
	const char* basestr = "frequency = ";
	BufferString suffixstr = zIsTime() ? " Hz" : " cycles/mm";
	for ( float freq=step_; freq<fnyq; freq+=step_ )
	{
		BufferString tmpstr = basestr; tmpstr += freq; tmpstr += suffixstr;
		nms.add( tmpstr.buf() );
	}
}
	
bool RSpecAttrib::prepPriorToOutputSetup()
{
	return areAllOutputsEnabled();
}
	
void rspec4single(const Eigen::ArrayXd& input, double dt, double omega, double freq, Eigen::ArrayXd& spec)
{
    int ns = input.size();
    spec.resize(ns);
    double sigma_p = (sqrt(omega)*6)/(sqrt(2*M_PI*dt)*27.0*exp(-3.0));
    double sigma_pT = sigma_p*dt;
    double sigma_pT2 = sigma_pT*sigma_pT;
    double sigma_pT3 = sigma_pT2*sigma_pT;
    double sigma_pT4 = sigma_pT3*sigma_pT;
        
    Eigen::Array4cd num;
    Eigen::Array4cd den;
    Eigen::ArrayXcd cinput = input;
    Eigen::ArrayXcd work(ns);
    double omega_p = 2.0*M_PI*freq;
    std::complex<double> p(-sigma_p, omega_p);
    std::complex<double> a = exp(p*dt);
    std::complex<double> a2 = a*a;
    std::complex<double> a3 = a2*a;
    std::complex<double> a4 = a3*a;
            
    num << 0.0, sigma_pT4*a/6.0, sigma_pT4*a2*2.0/3.0, sigma_pT4*a3/6.0;
    den << -4.0*a, 6.0*a2, -4.0*a3, a4;
    EigenFilter::iirfiltfilt(cinput, num, den, work);
    spec =  abs(work);
}
    
void rrspec4(const Eigen::ArrayXd& input, double dt, double fstep, const Eigen::ArrayXd& freq, Eigen::ArrayXXd& spec)
{
    int ns = input.size();
    int nf = freq.size();
    spec.resize(ns,nf);
    spec.setZero();
    double sigma_p = (sqrt(fstep)*6)/(sqrt(2*M_PI*dt)*27.0*exp(-3.0));
    double sigma_pT = sigma_p*dt;
    double sigma_pT2 = sigma_pT*sigma_pT;
    double sigma_pT3 = sigma_pT2*sigma_pT;
    double sigma_pT4 = sigma_pT3*sigma_pT;
    double sigma_p4T3 = sigma_p*sigma_pT3;
    double sigma_p4T5 = dt*sigma_pT4;
    
    Eigen::ArrayXcd num(4), dnum(4);
    Eigen::ArrayXcd den(4), dden(4);
    Eigen::ArrayXcd tnum(5);
    Eigen::ArrayXcd tden(5);
    Eigen::ArrayXcd cinput = input;
    Eigen::ArrayXcd tmp(ns);
    Eigen::ArrayXXcd w(ns, nf);
    Eigen::ArrayXXcd w_d(ns, nf);
    Eigen::ArrayXXcd w_t(ns, nf);
    
    for (int ifreq=0; ifreq<nf; ifreq++) {
        double omega_p = 2.0*M_PI*freq[ifreq];
        std::complex<double> p(-sigma_p, omega_p);
        std::complex<double> pT = p*dt;
        std::complex<double> a = exp(p*dt);
        std::complex<double> a2 = a*a;
        std::complex<double> a3 = a2*a;
        std::complex<double> a4 = a3*a;
            
        num << 0.0, sigma_pT4*a/6.0, sigma_pT4*a2*2.0/3.0, sigma_pT4*a3/6.0;
        den << -4.0*a, 6.0*a2, -4.0*a3, a4;
        dnum << 0.0, sigma_p4T3*a*(3.0+pT)/6.0, sigma_p4T3*a2*pT*2.0/3.0, -sigma_p4T3*a3*(3.0-pT)/6.0;
        dden = den;
        tnum << 0.0, sigma_p4T5*a/6.0, sigma_p4T5*a2*11.0/6.0, sigma_p4T5*a3*11.0/6.0, sigma_p4T5*a4/6.0;
        tden << -5.0*a, 10.0*a2, -10.0*a3, 5.0*a4, -a4*a;
        EigenFilter::iirfilt(cinput, num, den, tmp);
        w.col(ifreq) = tmp;
        EigenFilter::iirfilt(cinput, dnum, dden, tmp);
        w_d.col(ifreq) = tmp;
        EigenFilter::iirfilt(cinput, tnum, tden, tmp);
        w_t.col(ifreq) = tmp;
    }
    Eigen::ArrayXXd rtfr = abs(w);
    w += std::complex<double>(mDefEps, mDefEps);
    w_t /= w;
    w_d /= w;
    double f0 = freq[0];
    for (int idx=0; idx<ns; idx++) {
        for (int ifreq=0; ifreq<nf; ifreq++) {
            int that = idx-mNINT32(real(w_t(idx, ifreq))/dt);
            that = mMIN(mMAX(that, 0), ns-1);
            
            int fhat = mNINT32((imag(w_d(idx,ifreq))/M_2PI - f0)/fstep);
            fhat = mMIN(mMAX(fhat, 0), nf-1);
            spec(that, fhat) += rtfr(idx, ifreq);
        }
    }
}

bool RSpecAttrib::computeData( const DataHolder& output, const BinID& relpos, int z0, int nrsamples, int threadid ) const
{
    if ( !indata_ || indata_->isEmpty() || output.isEmpty() )
        return false;
    const int sz = zsampMargin_.width() + nrsamples;
    const int nfreq = outputinterest_.size();
    
    Eigen::ArrayXd trc(sz);
    for (int idx=0; idx<sz; idx++) {
        float val = getInputValue(*indata_, indataidx_, idx+zsampMargin_.start, z0);
        val = mIsUdf(val)?0.0f:val;
        trc[idx] = val;
    }

    int off = zsampMargin_.start-mNINT32((gate_.start + gate_.stop)/2.0/refstep_);
    if (reassign_) {
        Eigen::ArrayXd freq = Eigen::ArrayXd::LinSpaced(nfreq, step_, nfreq*step_);
        Eigen::ArrayXXd spec(sz, nfreq);
        rrspec4(trc, getRefStep(), step_, freq, spec);
        for (int idf=0; idf<nfreq; idf++) {
            if (!outputinterest_[idf])
                continue;
            for (int idx=0; idx<nrsamples; idx++)
                setOutputValue( output, idf, idx, z0, spec(idx-off, idf));
        }
        
    } else {
        Eigen::ArrayXd spec(sz);
        for (int idf=0; idf<nfreq; idf++) {
            if (!outputinterest_[idf])
                continue;
            double freq = (idf+1)*step_;
            rspec4single(trc, getRefStep(), 2.0/window_, freq, spec);
            for (int idx=0; idx<nrsamples; idx++)
                setOutputValue( output, idf, idx, z0, spec(idx-off));
        }
    }
    return true;
}


} // namespace Attrib

