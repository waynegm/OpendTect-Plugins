/*
 *   EFDAttrib Plugin - Empirical Fourier Mode Decomposition Modes Attribute
 *   Copyright (C) 2021  Wayne Mogg
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


#include "efdmodesattrib.h"
#include "attribdataholder.h"
#include "attribdesc.h"
#include "attribfactory.h"
#include "attribparam.h"
#include "paralleltask.h"
#include "survinfo.h"

#include "efd.h"

namespace Attrib
{

mAttrDefCreateInstance(EFDModesAttrib)

void EFDModesAttrib::initClass()
{
    mAttrStartInitClassWithDescAndDefaultsUpdate

    IntParam* nrmodes = new IntParam( nrmodesStr() );
    nrmodes->setLimits( 1, 20 );
    nrmodes->setDefaultValue( 5 );
    desc->addParam( nrmodes );

    desc->addOutputDataType( Seis::UnknowData );
    desc->addInput( InputSpec("Input Volume",true) );

    desc->setLocality( Desc::SingleTrace );
    mAttrEndInitClass
}

void EFDModesAttrib::updateDesc( Desc& desc )
{
    int nmodes;
    mGetIntFromDesc( desc, nmodes, nrmodesStr() );

    desc.setNrOutputs( Seis::UnknowData, nmodes );
}

void EFDModesAttrib::updateDefaults( Desc& desc )
{
}

EFDModesAttrib::EFDModesAttrib( Desc& desc )
    : Provider( desc )
{
    if ( !isOK() ) return;

    mGetInt( nrmodes_, nrmodesStr() );

    dessampgate_ =  Interval<int>( -(1024-1), 1024-1 );
    efdobj_ = new EFD(nrmodes_, EFD::OneSided, false, 0);
}

bool EFDModesAttrib::getInputData( const BinID& relpos, int zintv )
{
	indata_ = inputs_[0]->getData( relpos, zintv );
	if ( !indata_ )
		return false;

	indataidx_ = getDataIndex( 0 );

    return true;
}

bool EFDModesAttrib::areAllOutputsEnabled() const
{
    for (int idx=0; idx<nrOutputs(); idx++)
    {
        if (!outputinterest_[idx])
            return false;
    }
    return true;
}

void EFDModesAttrib::getCompNames( BufferStringSet& nms ) const
{
    nms.erase();
    for ( int idx=1; idx<=nrmodes_; idx++ ) {
        BufferString tmpstr("mode = ");
	tmpstr += idx;
        nms.add( tmpstr.buf() );
    }
}

bool EFDModesAttrib::prepPriorToOutputSetup()
{
    return areAllOutputsEnabled();
}

bool EFDModesAttrib::computeData( const DataHolder& output, const BinID& relpos,
			   int z0, int nrsamples, int threadid ) const
{
    if ( !indata_ || indata_->isEmpty() || output.isEmpty() )
        return false;
    const int ns = nrsamples;
    const int nmodes = outputinterest_.size();

    Eigen::ArrayXf input(ns);
    for (int idx=0; idx<ns; idx++) {
	float val = getInputValue(*indata_, indataidx_, idx, z0);
	input[idx] = mIsUdf(val)?0.0f:val;
    }

    efdobj_->setInput(input);
    for (int imode=0; imode<nmodes; imode++) {
        if (!outputinterest_[imode])
            continue;

	input = efdobj_->getMode(imode);
	for (int idx=0; idx<nrsamples; idx++) {
            float outVal = input(idx);
            setOutputValue( output, imode, idx, z0, outVal );
        }
    }
    return true;
}


} // namespace Attrib

