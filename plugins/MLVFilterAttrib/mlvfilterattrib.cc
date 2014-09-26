/*Copyright (C) 2014 Wayne Mogg All rights reserved.

This file may be used either under the terms of:

1. The GNU General Public License version 3 or higher, as published by
the Free Software Foundation,

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

/*+
________________________________________________________________________

 Author:        Wayne Mogg
 Date:          January 2014
________________________________________________________________________

-*/

#include "mlvfilterattrib.h"

#include "arrayndimpl.h"
#include "attribdataholder.h"
#include "attribdesc.h"
#include "attribdescset.h"
#include "attribfactory.h"
#include "attribparam.h"
#include "attribsteering.h"
#include <math.h>

namespace Attrib
{

mAttrDefCreateInstance(MLVFilter)    
    

void MLVFilter::initClass()
{
    mAttrStartInitClass

    IntParam* sizepar = new IntParam( sizeStr() );
    sizepar->setLimits( StepInterval<int>(3,9,2) );
    sizepar->setDefaultValue( 3 );
    desc->addParam( sizepar );

    EnumParam* out_type = new EnumParam( outputStr() );
    out_type->addEnum( "Average" );
    out_type->addEnum( "Median" );
    out_type->addEnum( "Element" );
    out_type->setDefaultValue( MLVFilter::Average );
    desc->addParam( out_type );

    desc->addInput( InputSpec("Input data",true) );
    desc->addOutputDataType( Seis::UnknowData );

    desc->setLocality( Desc::MultiTrace );
    mAttrEndInitClass
}


MLVFilter::MLVFilter( Desc& desc )
    : Provider( desc )
    , stepout_(0,0)
    , dessampgate_(0,0)
{
    if ( !isOK() ) return;

    inputdata_.allowNull(true);

    mGetEnum( outtype_, outputStr() );
    mGetInt( size_, sizeStr() );

    if ( size_%2 == 0 )
      size_++;
	const int hsz = size_/2;
    stepout_ = is2D() ? BinID(0,hsz) : BinID(hsz,hsz);

    getTrcPos();
    dessampgate_ = Interval<int>(-hsz, hsz);
    getElements();
    inputdata_.allowNull( true );
}


bool MLVFilter::getTrcPos()
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

bool MLVFilter::getElements()
{
	elements_.erase();
	const int nrelem = 13;
	Array1DImpl<PlanePoint> pnorms(nrelem);
	pnorms.set(0, PlanePoint(  0,  0,  1 ));
	pnorms.set(1, PlanePoint(  0,  1, -1 ));
	pnorms.set(2, PlanePoint(  0,  1,  1 ));
	pnorms.set(3, PlanePoint(  1,  0, -1 ));
	pnorms.set(4, PlanePoint(  1,  0,  1 ));
	pnorms.set(5, PlanePoint(  1,  1,  1 ));
	pnorms.set(6, PlanePoint(  1,  1, -1 ));
	pnorms.set(7, PlanePoint(  1, -1,  1 ));
	pnorms.set(8, PlanePoint( -1,  1,  1 ));
	pnorms.set(9, PlanePoint(  0,  1,  0 ));
	pnorms.set(10, PlanePoint(  1,  0,  0 ));
	pnorms.set(11, PlanePoint(  1, -1,  0 ));
	pnorms.set(12, PlanePoint( -1, -1,  0 ));
	
	for ( int ielem=0; ielem<nrelem; ielem++ ) {
		SamplePointSet pset; 
		for ( int trcidx=0; trcidx<trcpos_.size(); trcidx++ ) {
			const BinID bid = trcpos_[trcidx];
			for ( int zidx=dessampgate_.start; zidx<=dessampgate_.stop; zidx++ ) {
					PlanePoint pos( bid.inl(), bid.crl(), zidx);
					SamplePoint samp( trcidx, zidx );
					if (pos.dot(pnorms[ielem]) == 0 )
						pset += samp;
			}
		}
		elements_ += pset;
	}
	return true;
}


bool MLVFilter::getInputData( const BinID& relpos, int zintv )
{
	while ( inputdata_.size() < trcpos_.size() )
		inputdata_ += 0;
	
	const BinID bidstep = inputs_[0]->getStepoutStep();
	for ( int idx=0; idx<trcpos_.size(); idx++ )
	{
		const DataHolder* data = 
		inputs_[0]->getData( relpos+trcpos_[idx]*bidstep, zintv );
		if ( !data ) return false;
		inputdata_.replace( idx, data );
	}
	
	dataidx_ = getDataIndex( 0 );
	
	return true;
}

int MLVFilter::findLeastVarianceElement( int idx, int z0, float* result,  wmStats::StatCalc<float>& elemStats) const
{
	int minelem = 0;
	float minvar = -1;
	for (int elem=0; elem<elements_.size(); elem++) {
		SamplePointSet points = elements_[elem];
		const int np = points.size();
		elemStats.clear();
		for ( int ipnt=0; ipnt < np; ipnt++ ) {
			int trcidx = points[ipnt].x;
			const DataHolder* data = inputdata_[trcidx];
			const int zidx = points[ipnt].y;
			float traceval = mUdf(float);
			if ( data )
				traceval = getInputValue( *data, dataidx_, idx + zidx, z0);
			elemStats.addValue(traceval);
		}
		if (np) {
			float variance = elemStats.variance();
			if ( variance<minvar || minvar<0) {
				minvar = variance;
				minelem = elem;
				if ( outtype_ == Average ) {
					*result = elemStats.average();
				} else if (outtype_ == Median ) {
					*result = elemStats.median();
				}
			}
			if (mIsZero(variance,mDefEpsF)) break;
		}
	}
	return minelem;
}

bool MLVFilter::computeData( const DataHolder& output, const BinID& relpos, 
				int z0, int nrsamples, int threadid ) const
{
	
	if ( inputdata_.isEmpty() ) return false;
	
	wmStats::StatCalc<float> elemStats;
	float value=0;
	int elem=0;
	for ( int idx=0; idx<nrsamples; idx++ )
	{
		elem = findLeastVarianceElement( idx, z0, &value, elemStats ); 
		if ( outtype_ == Element ) {
			setOutputValue( output, 0, idx, z0, (float) elem );
		} else {
			setOutputValue( output, 0, idx, z0, value );
		}
	}
	return true;
}


const BinID* MLVFilter::desStepout( int inp, int out ) const
{ return inp ? 0 : &stepout_; }









}; //namespace

