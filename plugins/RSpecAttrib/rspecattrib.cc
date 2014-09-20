/*Copyright (C) 2014 Wayne Mogg All rights reserved.
 * 
 * This file may be used either under the terms of:
 * 
 * 1. The GNU General Public License version 3 or higher, as published by
 * the Free Software Foundation, or
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/*+
 * ________________________________________________________________________
 * 
 * Author:        Wayne Mogg
 * Date:          August 2014
 * ________________________________________________________________________
 * 
 * -*/


#include "rspecattrib.h"
#include "attribdataholder.h"
#include "attribdesc.h"
#include "attribfactory.h"
#include "attribparam.h"
#include "survinfo.h"

#include "commondefs.h"
#include "odcomplex.h"
#include "arrayndimpl.h"
#include "math2.h"
#include "mymath.h"


namespace Attrib
{
mAttrDefCreateInstance(RSpecAttrib)    
    
void RSpecAttrib::initClass()
{
	mAttrStartInitClassWithUpdate

	FloatParam* window = new FloatParam( windowStr() );
	window->setLimits(0.0f,400.0f);
	window->setDefaultValue( 56.0f);
	desc->addParam( window );

	FloatParam* step = new FloatParam( stepStr() );
	step->setRequired( false );
	step->setDefaultValue( 5.0f);
	desc->addParam( step );
	
//	FloatParam* freq = new FloatParam( freqStr() );
//    freq->setDefaultValue( 30.0f);
//    desc->addParam( freq );

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


RSpecAttrib::RSpecAttrib( Desc& desc )
    : Provider( desc )
	, order_(4)
{
    if ( !isOK() ) return;

    mGetFloat( window_, windowStr() );
	mGetFloat( step_, stepStr() );
	
	window_ = window_/zFactor();
	float refstep = getRefStep();
	zsampMargin_ = Interval<int>(mNINT32(-window_/refstep)-1, mNINT32(window_/refstep)+1);
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
	
void RSpecAttrib::computeFilterTaps(float freq, Array1DImpl<float_complex>& num, Array1DImpl<float_complex>& den ) const
{
	float refstep = getRefStep();
	float dfreq = 1.0f/window_;  
	int km1 = order_-1;
	float sigma_p = (sqrt(dfreq)*myMath::factorial(km1))/(sqrt(M_2PIf*refstep)*pow(km1,km1)*exp(-km1));
	float omega = M_2PIf*freq;
	float_complex p(-sigma_p, omega);
	float_complex a = exp(p*refstep);
	float sigma_pT = sigma_p * refstep;
	float s;
	
	switch (order_)
	{
		case 2:	
			s = sigma_pT * sigma_pT;
			num.set(0, 0);
			num.set(1, s * a);
			den.set(0, -2.0f * a);
			den.set(1, a * a);
			break;
		case 3:
			s = sigma_pT * sigma_pT * sigma_pT;
			num.set(0, 0);
			num.set(1, s * a / 2.0f);
			num.set(2, s * a * a / 2.0f);
			den.set(0, -3.0f * a);
			den.set(1, 3.0f * a * a);
			den.set(2, -a * a * a);
			break;
		case 4:
			s = sigma_pT * sigma_pT * sigma_pT * sigma_pT;
			num.set(0, 0);
			num.set(1, s * a / 6.0f);
			num.set(2, s * a * a * 2.0f / 3.0f);
			num.set(3, s * a * a * a / 6.0f);
			den.set(0, -4.0f * a);
			den.set(1, 6.0f * a * a);
			den.set(2, -4.0f * a * a * a);
			den.set(3, a * a * a * a);
			break;
		case 5:
			s = sigma_pT * sigma_pT * sigma_pT * sigma_pT * sigma_pT;
			num.set(0, 0);
			num.set(1, s * a / 24.0f);
			num.set(2, s * a * a * 11.0f / 24.0f);
			num.set(3, s * a * a * a * 11.0f / 24.0f);
			num.set(4, s * a * a * a * a / 24.0f);
			den.set(0, -5.0f * a);
			den.set(1, 10.0f * a * a);
			den.set(2, -10.0f * a * a * a);
			den.set(3, 5.0f * a * a * a * a);
			den.set(4, -a * a * a * a * a);
			break;
	}	
}

void RSpecAttrib::computeFrequency(Array1DImpl<float_complex>& num, Array1DImpl<float_complex>& den,
								   int nrsamples, Array1DImpl<float>& trc, Array1DImpl<float>& spec ) const
{
	Array1DImpl<float_complex> work(nrsamples);
	Array1DImpl<float_complex> workr(nrsamples);
	const float_complex czero(0.0f,0.0f);
	float_complex tmp;
	work.set(0, czero);
// Forward Pass
	for (int i=0; i<nrsamples;i++) {
		tmp = czero;
		int nw = (order_<i+1)?order_:i+1;
		int j = 0;
		while (j<nw) {
			tmp += num[j] * trc[i-j];
			j++;
		}
		nw = (order_<i)?order_:i;
		j = 0;
		while (j<nw) {
			tmp -= den[j] * work[i-j-1];
			j++;
		}
		work.set(i,tmp);
	}
// Reverse Pass
	for (int i=nrsamples-1;i>=0;i--) {
		tmp = czero;
		int nw = (order_<nrsamples-1-i+1)?order_:nrsamples-1-i+1;
		int j = 0;
		while (j<nw) {
			tmp += conj(num[j]) * work[i+j];
			j++;
		}
		nw = (order_<nrsamples-1-i)?order_:nrsamples-1-i;
		j = 0;
		while (j<nw) {
			tmp -= conj(den[j]) * workr[i+j+1];
			j++;
		}	
		workr.set(i, tmp);
	}
	
	for (int i =0; i<nrsamples;i++)
		spec.set(i,abs(workr[i]));
}

bool RSpecAttrib::computeData( const DataHolder& output, const BinID& relpos,
			   int z0, int nrsamples, int threadid ) const
{
	if ( !indata_ || indata_->isEmpty() || output.isEmpty() )
		return false;
	const int sz = zsampMargin_.width() + nrsamples;
	const int nfreq = outputinterest_.size();
	Array1DImpl<float> trc(sz), spec(sz);
	Array1DImpl<float_complex> num(order_), den(order_);

	for (int idx=0; idx<sz; idx++) {
		float val = getInputValue(*indata_, indataidx_, zsampMargin_.start+idx, z0);
		val = mIsUdf(val)?0.0f:val;
		trc.set(idx,val);
	}
	
	for (int idf=0; idf<nfreq; idf++) {
		if (!outputinterest_[idf])
			continue;
		float freq = step_ * (idf+1);
		computeFilterTaps(freq, num, den);
		computeFrequency(num, den, sz, trc, spec);
		for (int idx=0; idx<nrsamples; idx++)
			setOutputValue( output, idf, idx, z0, spec[idx-zsampMargin_.start] );
	}
    return true;
}


} // namespace Attrib

