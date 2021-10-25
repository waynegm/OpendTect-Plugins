/*Copyright (C) 2015 Wayne Mogg All rights reserved.

This file may be used either under the terms of:

1. The GNU General Public License version 3 or higher, as published by
the Free Software Foundation,

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

/*+
________________________________________________________________________

 Author:        Wayne Mogg
 Date:          November 2015
________________________________________________________________________

-*/

#include "gradientattrib.h"

#include "arrayndimpl.h"
#include "attribdataholder.h"
#include "attribdesc.h"
#include "attribdescset.h"
#include "attribfactory.h"
#include "attribparam.h"
#include <math.h>

namespace Attrib
{

mAttrDefCreateInstance(GradientAttrib)


void GradientAttrib::initClass()
{
    mAttrStartInitClass

    EnumParam* op_type = new EnumParam( operatorStr() );
	op_type->addEnum("Kroon_3");
	op_type->addEnum("Farid_5");
	op_type->addEnum("Farid_7");
    op_type->setDefaultValue( GradientAttrib::Kroon_3 );
    desc->addParam( op_type );

    EnumParam* out_type = new EnumParam( outputStr() );
    out_type->addEnum( "Inline" );
    out_type->addEnum( "Crossline" );
    out_type->addEnum( "Z" );
    out_type->setDefaultValue( GradientAttrib::Z );
    desc->addParam( out_type );

    desc->addInput( InputSpec("Input data",true) );
    desc->addOutputDataType( Seis::UnknowData );

    desc->setLocality( Desc::MultiTrace );
    mAttrEndInitClass
}

const float GradientAttrib::kroon_3_d[] = { -0.500000,  0.000000,  0.500000 };
const float GradientAttrib::kroon_3_s[] = {  0.178947,  0.642105,  0.178947 };
const float GradientAttrib::farid_5_d[] = { -0.109604, -0.276691,  0.000000, 0.276691, 0.109604 };
const float GradientAttrib::farid_5_s[] = {  0.037659,  0.249153,  0.426375, 0.249153, 0.037659 };
const float GradientAttrib::farid_7_d[] = { -.018708, -0.125376, -0.193091, 0.000000, 0.193091, 0.125376, 0.018708 };
const float GradientAttrib::farid_7_s[] = {  0.004711,  0.069321,  0.245410, 0.361117, 0.245410, 0.069321, 0.004711 };


GradientAttrib::GradientAttrib( Desc& desc )
    : Provider( desc )
	, size_(0)
    , stepout_(0,0)
    , zmargin_(0,0)
{
    if ( !isOK() ) return;

    inputdata_.allowNull(true);

    mGetEnum( outtype_, outputStr() );
    mGetEnum( optype_, operatorStr() );
	float*	skernel = NULL;
	float*	dkernel = NULL;

	switch(optype_)
	{
		case Kroon_3:	size_ = 3;
						skernel = new float[size_];
						dkernel = new float[size_];
						OD::memCopy( skernel, kroon_3_s, size_*sizeof(float) );
						OD::memCopy( dkernel, kroon_3_d, size_*sizeof(float) );
						break;
		case Farid_5:	size_ = 5;
						skernel = new float[size_];
						dkernel = new float[size_];
						OD::memCopy( skernel, farid_5_s, size_*sizeof(float) );
						OD::memCopy( dkernel, farid_5_d, size_*sizeof(float) );
						break;
		case Farid_7:	size_ = 7;
						skernel = new float[size_];
						dkernel = new float[size_];
						OD::memCopy( skernel, farid_7_s, size_*sizeof(float) );
						OD::memCopy( dkernel, farid_7_d, size_*sizeof(float) );
						break;
	};

	ikernel_ = new float[size_];
	xkernel_ = new float[size_];
	zkernel_ = new float[size_];
	OD::memCopy( ikernel_, skernel, size_*sizeof(float) );
	OD::memCopy( xkernel_, skernel, size_*sizeof(float) );
	OD::memCopy( zkernel_, skernel, size_*sizeof(float) );
	switch(outtype_)
	{
		case Inline:	OD::memCopy( ikernel_, dkernel, size_*sizeof(float) );
						break;
		case Crossline:	OD::memCopy( xkernel_, dkernel, size_*sizeof(float) );
						break;
		case Z:			OD::memCopy( zkernel_, dkernel, size_*sizeof(float) );
						break;
	};
	delete [] skernel;
	delete [] dkernel;

	const int hsz = size_/2;

    stepout_ = is2D() ? BinID(0,hsz) : BinID(hsz,hsz);
    getTrcPos();
    zmargin_ = Interval<int>(-hsz, hsz);
    inputdata_.allowNull( true );
}

GradientAttrib::~GradientAttrib()
{
	delete [] ikernel_;
	delete [] xkernel_;
	delete [] zkernel_;
}

bool GradientAttrib::getTrcPos()
{
    trcpos_.erase();
    BinID bid;
    int trcidx = 0;
    centertrcidx_ = 0;
    for ( bid.inl()=-stepout_.inl(); bid.inl()<=stepout_.inl(); bid.inl()++ )
    {
		for ( bid.crl()=-stepout_.crl(); bid.crl()<=stepout_.crl(); bid.crl()++ )
		{
			if ( !bid.inl() && !bid.crl() )
				centertrcidx_ = trcidx;
			trcpos_ += bid;
			trcidx++;
		}
    }

    return true;
}

bool GradientAttrib::getInputData( const BinID& relpos, int zintv )
{
	while ( inputdata_.size() < trcpos_.size() )
		inputdata_ += 0;

	const BinID bidstep = inputs_[0]->getStepoutStep();
	for ( int idx=0; idx<trcpos_.size(); idx++ )
	{
		const DataHolder* data =
		inputs_[0]->getData( relpos+trcpos_[idx]*bidstep, zintv );
        if ( !data ) {
            const BinID pos = relpos + trcpos_[centertrcidx_]*bidstep;
            data = inputs_[0]->getData( pos, zintv );
            if ( !data ) return false;
        }
		inputdata_.replace( idx, data );
	}

	dataidx_ = getDataIndex( 0 );

	return true;
}

bool GradientAttrib::computeData( const DataHolder& output, const BinID& relpos,
				  int z0, int nrsamples, int threadid ) const
{

	if ( inputdata_.isEmpty() ) return false;

	const int sz = zmargin_.width() + nrsamples;
	Array1DImpl<float> vals( sz );
	const int hsz = size_/2;

	for ( int idx=0; idx<sz; idx++ ) {
		float reso = 0.0;
		for (int iln=0; iln<size_; iln++) {
			float res = 0.0;
			for (int crl=0; crl<size_; crl++) {
				const DataHolder* data = inputdata_[iln*size_+crl];
				float val = getInputValue(*data, dataidx_, zmargin_.start+idx, z0);
				res += mIsUdf(val)?0.0f:val*xkernel_[crl];
			}
			reso += ikernel_[iln]*res;
		}
		vals.set(idx, reso);
	}

	for (int idx=0; idx<nrsamples; idx++) {
		float value = 0.0;
		const int ipos = idx - zmargin_.start - hsz;
		for (int zi=0; zi<size_; zi++) {
			value += vals[ipos+zi]*zkernel_[zi];
		}
		setOutputValue( output, 0, idx, z0, value );
	}
	return true;
}


const BinID* GradientAttrib::desStepout( int inp, int out ) const
{ return &stepout_; }


}; //namespace

