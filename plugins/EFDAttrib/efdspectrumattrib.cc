/*
 *   EFDAttrib Plugin - Empirical Fourier Decomposition Time-Frequency Attribute
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


#include "efdspectrumattrib.h"
#include "attribdataholder.h"
#include "attribdesc.h"
#include "attribfactory.h"
#include "attribparam.h"
#include "survinfo.h"

#include "efd.h"

namespace Attrib
{

mAttrDefCreateInstance(EFDSpectrumAttrib)

void EFDSpectrumAttrib::initClass()
{
    mAttrStartInitClassWithDescAndDefaultsUpdate

    IntParam* nrmodes = new IntParam( nrmodesStr() );
    desc->addParam( nrmodes );

    FloatParam* step = new FloatParam( stepStr() );
    step->setRequired( false );
    step->setDefaultValue( 5.0f);
    desc->addParam( step );

    desc->addOutputDataType( Seis::UnknowData );
    desc->addInput( InputSpec("Input Volume",true) );

    mAttrEndInitClass
}

void EFDSpectrumAttrib::updateDesc( Desc& desc )
{
    float dfreq;
    mGetFloatFromDesc( desc, dfreq, stepStr() );
    const float nyqfreq = 0.5f / SI().zStep();
    const int nrattribs = (int)( nyqfreq / dfreq );
    desc.setNrOutputs( Seis::UnknowData, nrattribs );
}

void EFDSpectrumAttrib::updateDefaults( Desc& desc )
{
}

EFDSpectrumAttrib::EFDSpectrumAttrib( Desc& desc )
    : Provider( desc )
{
    if ( !isOK() ) return;

    mGetInt( nrmodes_, nrmodesStr() );
    mGetFloat( step_, stepStr() );

    dessampgate_ =  Interval<int>( -(1024-1), 1024-1 );
    efdobj_ = new EFD(nrmodes_, EFD::OneSided, false, 0);
}

bool EFDSpectrumAttrib::getInputData( const BinID& relpos, int zintv )
{
	indata_ = inputs_[0]->getData( relpos, zintv );
	if ( !indata_ )
		return false;

	indataidx_ = getDataIndex( 0 );

    return true;
}

bool EFDSpectrumAttrib::areAllOutputsEnabled() const
{
    for (int idx=0; idx<nrOutputs(); idx++)
        if (!outputinterest_[idx])
            return false;
        return true;
}

void EFDSpectrumAttrib::getCompNames( BufferStringSet& nms ) const
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

bool EFDSpectrumAttrib::prepPriorToOutputSetup()
{
    return areAllOutputsEnabled();
}

bool EFDSpectrumAttrib::computeData( const DataHolder& output, const BinID& relpos,
			   int z0, int nrsamples, int threadid ) const
{
    if ( !indata_ || indata_->isEmpty() || output.isEmpty() )
        return false;
    const int ns = nrsamples;
    const int nfreq = outputinterest_.size();

    Eigen::ArrayXf input(ns);
    for (int idx=0; idx<ns; idx++) {
	float val = getInputValue(*indata_, indataidx_, idx, z0);
	input(idx) = mIsUdf(val)?0.0f:val;
    }

    efdobj_->setInput(input);
    Eigen::ArrayXXf freqdata = Eigen::ArrayXXf::Zero(nfreq, ns);
    for (int imode=0; imode<nrmodes_; imode++) {
	auto modespec = efdobj_->getModeHTInstFrequency(imode, refstep_);
//	modespec.row(1) = (modespec.row(1).isFinite()).select(modespec.row(1),0.f);
//	modespec.row(0) = (modespec.row(0).isFinite()).select(modespec.row(0),0.f);
	float minv = modespec.row(0).minCoeff();
	float maxv = modespec.row(0).maxCoeff();
	BufferString msg;
	msg.add("Min: ").add(minv).add(" Max: ").add(maxv);
	ErrMsg(msg);
	for (int idf=0; idf<nfreq; idf++) {
	    if (!outputinterest_[idf])
		continue;
	    const float freq = step_ * (idf+1);
	    freqdata.row(idf) += (modespec.row(0)>=freq-step_/2.f && modespec.row(0)<freq+step_/2).select(modespec.row(1), 0.f);
	}
    }
    for (int idf=0; idf<nfreq; idf++) {
	if (!outputinterest_[idf])
	    continue;
	for (int idx=0; idx<nrsamples; idx++) {
	    float outVal = freqdata(idf, idx);
	    setOutputValue( output, idf, idx, z0, outVal );
	}
    }
    return true;
}


} // namespace Attrib

