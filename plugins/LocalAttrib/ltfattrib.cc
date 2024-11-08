/*
 *   LocalAttrib Plugin
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


#include "ltfattrib.h"
#include "attribdataholder.h"
#include "attribdesc.h"
#include "attribfactory.h"
#include "attribparam.h"
#include "survinfo.h"

#include "arrayndimpl.h"
#include "math2.h"

#include "rsflib.h"
namespace Attrib
{

mAttrDefCreateInstance(LTFAttrib)

void LTFAttrib::initClass()
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

//    FloatParam* freq = new FloatParam( freqStr() );
//    freq->setDefaultValue( 30.0f);
//    desc->addParam( freq );

//    IntParam* smooth = new IntParam( smoothStr() );
//    desc->addParam( smooth );
//    smooth->setDefaultValue( 10 );

	IntParam* niter = new IntParam( niterStr() );
	desc->addParam( niter );
	niter->setDefaultValue( 100 );

//	IntParam* margin = new IntParam( marginStr() );
//	desc->addParam( margin );
//	margin->setDefaultValue( 4 );

	desc->addOutputDataType( Seis::UnknowData );
	desc->addInput( InputSpec("Input Volume",true) );

    mAttrEndInitClass
}

void LTFAttrib::updateDesc( Desc& desc )
{
    float dfreq;
    mGetFloatFromDesc( desc, dfreq, stepStr() );
    const float nyqfreq = 0.5f / SI().zStep();
    const int nrattribs = (int)( nyqfreq / dfreq );
    desc.setNrOutputs( Seis::UnknowData, nrattribs );
}

void LTFAttrib::updateDefaults( Desc& desc )
{
    ValParam* paramgate = desc.getValParam(gateStr());
    mDynamicCastGet( ZGateParam*, zgate, paramgate )
    float roundedzstep = SI().zStep()*SI().zDomain().userFactor();
    if ( roundedzstep > 0 )
        roundedzstep = Math::Floor( roundedzstep );
    zgate->setDefaultValue( Interval<float>(-roundedzstep*5, roundedzstep*5) );
}

LTFAttrib::LTFAttrib( Desc& desc )
    : Provider( desc )
{
    if ( !isOK() ) return;

    mGetFloatInterval( gate_, gateStr() );
    gate_.start_ = gate_.start_ / zFactor();
    gate_.stop_ = gate_.stop_ / zFactor();

    mGetFloat( step_, stepStr() );

    window_ = gate_.stop_ - gate_.start_;
    float refstep = getRefStep();
    zsampMargin_ = Interval<int>(mNINT32((gate_.start_-window_/2.0)/refstep)-1,
				 mNINT32((gate_.stop_+window_/2.0)/refstep)+1);

//    mGetFloat( freq_, freqStr() );
//    mGetInt( smooth_, smoothStr() );
	mGetInt( niter_, niterStr() );
//	mGetInt( margin_, marginStr() );

//	dessamp_ = Interval<int>(-smooth_*margin_, smooth_*margin_);
}

bool LTFAttrib::getInputData( const BinID& relpos, int zintv )
{
	indata_ = inputs_[0]->getData( relpos, zintv );
	if ( !indata_ )
		return false;

	indataidx_ = getDataIndex( 0 );

    return true;
}

bool LTFAttrib::areAllOutputsEnabled() const
{
    for (int idx=0; idx<nrOutputs(); idx++)
        if (!outputinterest_[idx])
            return false;
        return true;
}

void LTFAttrib::getCompNames( BufferStringSet& nms ) const
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

bool LTFAttrib::prepPriorToOutputSetup()
{
    return areAllOutputsEnabled();
}

bool LTFAttrib::computeData( const DataHolder& output, const BinID& relpos,
			   int z0, int nrsamples, int threadid ) const
{
    if ( !indata_ || indata_->isEmpty() || output.isEmpty() )
        return false;
    int ns = zsampMargin_.width() + nrsamples;
    const int nfreq = outputinterest_.size();

    Array1DImpl<float> ss(ns), bs(ns), bc(ns), trc(ns), cc(ns);
    int smooth = mNINT32(window_/(2.0*getRefStep()));
    sf::Divn sfdivn(1,ns,&ns,&smooth,niter_);

    for (int idx=0; idx<ns; idx++) {
	float val = getInputValue(*indata_, indataidx_, zsampMargin_.start_+idx, z0);
	val = mIsUdf(val)?0.0f:val;
	trc.set(idx,val);
    }
    int off = zsampMargin_.start_-mNINT32((gate_.start_ + gate_.stop_)/2.0/getRefStep());

    for (int idf=0; idf<nfreq; idf++) {
        if (!outputinterest_[idf])
            continue;
        float freq = step_ * (idf+1);
        float w = freq * 2.0 * M_PIf;

        if (w == 0.) {
            for ( int idx=0; idx<ns; idx++ ) {
                ss.set(idx, 0.0);
                bc.set(idx, 0.5);
            }
            sfdivn.doDiv(trc.arr(), bc.arr(), cc.arr());
        } else {
            for (int idx=0; idx<ns; idx++) {
                float t = (z0 + zsampMargin_.start_ + idx) * refstep_;
                bs.set(idx, sinf(w*t));
                bc.set(idx, cosf(w*t));
            }
            sfdivn.doDiv(trc.arr(),bs.arr(),ss.arr());
            sfdivn.doDiv(trc.arr(),bc.arr(),cc.arr());
        }
        for (int idx=0; idx<nrsamples; idx++) {
            float outVal = mUdf(float);
            outVal = hypotf(ss[idx-off],cc[idx-off]);
            setOutputValue( output, idf, idx, z0, outVal );
        }
    }
    return true;
}


} // namespace Attrib

