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
 * Date:          April 2014
 * ________________________________________________________________________
 * 
 * -*/


#include "ltfattrib.h"
#include "attribdataholder.h"
#include "attribdesc.h"
#include "attribfactory.h"
#include "attribparam.h"

#include "arrayndimpl.h"
#include "math2.h"
#include "rsf.hh"


namespace Attrib
{

mAttrDefCreateInstance(LTFAttrib)    
    
void LTFAttrib::initClass()
{
	mAttrStartInitClassWithUpdate

    FloatParam* freq = new FloatParam( freqStr() );
    freq->setDefaultValue( 30.0f);
    desc->addParam( freq );

    IntParam* smooth = new IntParam( smoothStr() );
    desc->addParam( smooth );
    smooth->setDefaultValue( 10 );

	IntParam* niter = new IntParam( niterStr() );
	desc->addParam( niter );
	niter->setDefaultValue( 100 );

	IntParam* margin = new IntParam( marginStr() );
	desc->addParam( margin );
	margin->setDefaultValue( 4 );

	desc->addOutputDataType( Seis::UnknowData );
	desc->addInput( InputSpec("Input Volume",true) );

    mAttrEndInitClass
}

void LTFAttrib::updateDesc( Desc& desc )
{
}

LTFAttrib::LTFAttrib( Desc& desc )
    : Provider( desc )
{
    if ( !isOK() ) return;

    mGetFloat( freq_, freqStr() );
    mGetInt( smooth_, smoothStr() );
	mGetInt( niter_, niterStr() );
	mGetInt( margin_, marginStr() );
	
	dessamp_ = Interval<int>(-smooth_*margin_, smooth_*margin_);
}


bool LTFAttrib::getInputData( const BinID& relpos, int zintv )
{
	indata_ = inputs_[0]->getData( relpos, zintv );
	if ( !indata_ )
		return false;

	indataidx_ = getDataIndex( 0 );
	
    return true;
}


bool LTFAttrib::computeData( const DataHolder& output, const BinID& relpos,
			   int z0, int nrsamples, int threadid ) const
{
	int ns = dessamp_.width() + nrsamples;
	Array1DImpl<float> ss(ns), bs(ns), bc(ns), trc(ns), cc(ns);
	int smooth = smooth_;
	sf_divn_init(1,ns,&ns,&smooth,niter_,false);
	
	float w = freq_ * 2.0 * M_PIf;
	
	for (int idx=0; idx<ns; idx++) {
		float val = getInputValue(*indata_, indataidx_, dessamp_.start+idx, z0);
		val = mIsUdf(val)?0.0f:val;
		trc.set(idx,val);
	}
	
	if (w == 0.) {
		for ( int idx=0; idx<ns; idx++ ) {
			ss.set(idx, 0.0);
			bc.set(idx, 0.5);
		}
		sf_divn( trc.arr(), bc.arr(), cc.arr() );
	} else {
		for (int idx=0; idx<ns; idx++) {
			float t = (z0 + dessamp_.start + idx) * refstep_;
			bs.set(idx, sinf(w*t));
			bc.set(idx, cosf(w*t));
		}
		sf_divn(trc.arr(),bs.arr(),ss.arr());
		sf_divn(trc.arr(),bc.arr(),cc.arr());
	}
	for (int idx=0; idx<nrsamples; idx++) {
		float outVal = mUdf(float);
		outVal = hypotf(ss[idx-dessamp_.start],cc[idx-dessamp_.start]);
		setOutputValue( output, 0, idx, z0, outVal );
	}
	
	sf_divn_close();
    return true;
}


} // namespace Attrib

