/*
 *   VMDAttrib Plugin - Variational Mode Decomposition Time-Frequency Attribute
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


#include "vmdspectrumattrib.h"
#include "attribdataholder.h"
#include "attribdesc.h"
#include "attribfactory.h"
#include "attribparam.h"
#include "survinfo.h"

#include "vmd.h"

namespace Attrib
{

mAttrDefCreateInstance(VMDSpectrumAttrib)

void VMDSpectrumAttrib::initClass()
{
    mAttrStartInitClassWithDescAndDefaultsUpdate

    IntParam* nmodes = new IntParam( modesStr() );
    nmodes->setLimits( 1, 10 );
    nmodes->setDefaultValue( 5 );
    desc->addParam( nmodes );

    FloatParam* step = new FloatParam( stepStr() );
    step->setRequired( false );
    step->setDefaultValue( 5.0f);
    desc->addParam( step );

    desc->addOutputDataType( Seis::UnknowData );
    desc->addInput( InputSpec("Input Volume",true) );

    mAttrEndInitClass
}

void VMDSpectrumAttrib::updateDesc( Desc& desc )
{
    float dfreq;
    mGetFloatFromDesc( desc, dfreq, stepStr() );
    const float nyqfreq = 0.5f / SI().zStep();
    const int nrattribs = (int)( nyqfreq / dfreq );
    desc.setNrOutputs( Seis::UnknowData, nrattribs );
}

void VMDSpectrumAttrib::updateDefaults( Desc& desc )
{
}

VMDSpectrumAttrib::VMDSpectrumAttrib( Desc& desc )
    : Provider( desc )
{
    if ( !isOK() ) return;

    mGetInt( nmodes_, modesStr() );

    mGetFloat( step_, stepStr() );

    float refstep = getRefStep();
}

bool VMDSpectrumAttrib::getInputData( const BinID& relpos, int zintv )
{
	indata_ = inputs_[0]->getData( relpos, zintv );
	if ( !indata_ )
		return false;

	indataidx_ = getDataIndex( 0 );

    return true;
}

bool VMDSpectrumAttrib::areAllOutputsEnabled() const
{
    for (int idx=0; idx<nrOutputs(); idx++)
        if (!outputinterest_[idx])
            return false;
        return true;
}

void VMDSpectrumAttrib::getCompNames( BufferStringSet& nms ) const
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

bool VMDSpectrumAttrib::prepPriorToOutputSetup()
{
    return areAllOutputsEnabled();
}

bool VMDSpectrumAttrib::computeData( const DataHolder& output, const BinID& relpos,
			   int z0, int nrsamples, int threadid ) const
{
    if ( !indata_ || indata_->isEmpty() || output.isEmpty() )
        return false;
    const int ns = nrsamples;
    const int nfreq = outputinterest_.size();

    Eigen::ArrayXd input(ns);
    for (int idx=0; idx<ns; idx++) {
	float val = getInputValue(*indata_, indataidx_, idx, z0);
	input(idx) = mIsUdf(val)?0.0f:val;
    }


    for (int idf=0; idf<nfreq; idf++) {
        if (!outputinterest_[idf])
            continue;
        float freq = step_ * (idf+1);
        float w = freq * 2.0 * M_PIf;

        for (int idx=0; idx<nrsamples; idx++) {
            float outVal = mUdf(float);
            setOutputValue( output, idf, idx, z0, outVal );
        }
    }
    return true;
}


} // namespace Attrib

