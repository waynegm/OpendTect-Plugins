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
 Date:          December 2014
________________________________________________________________________

-*/
#include "externalattrib.h"
#include "wmparamsetget.h"
#include "attribdataholder.h"
#include "attribdesc.h"
#include "attribdescset.h"
#include "attribfactory.h"
#include "attribparam.h"

#include "extproc.h"


namespace Attrib
{

mAttrDefCreateInstance(ExternalAttrib)    

ExtProc* ExternalAttrib::dProc_ = NULL;

void ExternalAttrib::initClass()
{
	mAttrStartInitClassWithUpdate

	StringParam* interpfilepar = new StringParam( interpFileStr() );
	desc->addParam( interpfilepar );
	
	StringParam* exfilepar = new StringParam( exFileStr() );
	desc->addParam( exfilepar );

	IntGateParam* zMargin = new IntGateParam( zmarginStr() );
	zMargin->setLimits( Interval<int>(-1000,1000) );
	zMargin->setDefaultValue( Interval<int>(-10, 10) );
	desc->addParam( zMargin );
	
	BinIDParam* stepout = new BinIDParam( stepoutStr() );
	stepout->setDefaultValue( BinID(0,0) );
	desc->addParam( stepout );

	
	FloatParam* par0 = new FloatParam( par0Str() );
	desc->addParam( par0 );
	par0->setDefaultValue(0);

	FloatParam* par1 = new FloatParam( par1Str() );
	desc->addParam( par1 );
	par1->setDefaultValue(0);
	
	FloatParam* par2 = new FloatParam( par2Str() );
	desc->addParam( par2 );
	par2->setDefaultValue(0);
	
	FloatParam* par3 = new FloatParam( par3Str() );
	desc->addParam( par3 );
	par3->setDefaultValue(0);
	
	FloatParam* par4 = new FloatParam( par4Str() );
	desc->addParam( par4 );
	par4->setDefaultValue(0);
	
	desc->addInput( InputSpec("Input data",true) );
	desc->addOutputDataType( Seis::UnknowData );
	
	mAttrEndInitClass
}

void ExternalAttrib::updateDesc( Desc& desc )
{

	BufferString iname = desc.getValParam( interpFileStr() )->getStringValue();
	BufferString fname = desc.getValParam( exFileStr() )->getStringValue();
    if (dProc_) {
        if (dProc_->getFile() != fname) {
            delete dProc_;
            dProc_ = new ExtProc(fname.str(), iname.str());
        }
    } else
        dProc_ = new ExtProc(fname.str(), iname.str());
	
	desc.setParamEnabled(zmarginStr(), dProc_->hasZMargin());
	desc.setParamEnabled(stepoutStr(), dProc_->hasStepOut());
	desc.setParamEnabled(par0Str(), dProc_->hasParam(0));
	desc.setParamEnabled(par1Str(), dProc_->hasParam(1));
	desc.setParamEnabled(par2Str(), dProc_->hasParam(2));
	desc.setParamEnabled(par3Str(), dProc_->hasParam(3));
	desc.setParamEnabled(par4Str(), dProc_->hasParam(4));
	
	if (dProc_->hasOutput())
		desc.setNrOutputs(Seis::UnknowData, dProc_->numOutput());
	else
		desc.setNrOutputs(Seis::UnknowData, 1);
	
	if (dProc_->hasStepOut())
		desc.setLocality(Desc::MultiTrace);
	else
		desc.setLocality(Desc::SingleTrace);
}

ExternalAttrib::ExternalAttrib( Desc& desc )
    : Provider( desc )
	, stepout_(0,0)
	, zmargin_(0,0)
{
	if ( !isOK() ) return;

	indata_.allowNull(true);
	enableAllOutputs(true);
	
	mGetString( interpfile_, interpFileStr() );
	mGetString( exfile_, exFileStr() );
	proc_= new ExtProc( exfile_.str(), interpfile_.str() );
	
	if (proc_) {
		if (proc_->hasZMargin()) {
			mGetIntInterval( zmargin_, zmarginStr() );
			proc_->setZMargin(zmargin_);
		}
		if (proc_->hasStepOut()) {
			mGetBinID( stepout_, stepoutStr() )
			proc_->setStepOut(stepout_);
		} else {
			stepout_ = BinID(0,0);
		}
		if (proc_->hasParam(0)) {
			mGetFloat(par0_, par0Str());
			proc_->setParam(0, par0_);
		}
		if (proc_->hasParam(1)) {
			mGetFloat(par1_, par1Str());
			proc_->setParam(1, par1_);
		}
		if (proc_->hasParam(2)) {
			mGetFloat(par2_, par2Str());
			proc_->setParam(2, par2_);
		}
		if (proc_->hasParam(3)) {
			mGetFloat(par3_, par3Str());
			proc_->setParam(3, par3_);
		}
		if (proc_->hasParam(4)) {
			mGetFloat(par4_, par4Str());
			proc_->setParam(4, par4_);
		}
		getTrcPos();
		int ninl = stepout_.inl()*2 + 1;
		int ncrl = stepout_.crl()*2 + 1;
		proc_->start(ninl, ncrl);
	} else 
		ErrMsg("ExternalAttrib::ExternalAttrib - error creating extrenal procedure");
}

ExternalAttrib::~ExternalAttrib()
{
	if (proc_)
		delete proc_;
}

bool ExternalAttrib::getTrcPos()
{
	trcpos_.erase();
	BinID bid;
	for ( bid.inl()=-stepout_.inl(); bid.inl()<=stepout_.inl(); bid.inl()++ )
		for ( bid.crl()=-stepout_.crl(); bid.crl()<=stepout_.crl(); bid.crl()++ )
			trcpos_ += bid;
	return true;
}

void ExternalAttrib::getCompNames( BufferStringSet& nms ) const
{
	if (proc_->hasOutput()) {
		nms.erase();
		for (int i = 0; i<proc_->numOutput();i++)
		{
			BufferString nm = proc_->outputName(i);
			nms.add(nm.buf());
		}
	} else 
		Provider::getCompNames( nms ); 
}

bool ExternalAttrib::getInputData( const BinID& relpos, int zintv )
{
	while ( indata_.size() < trcpos_.size() )
		indata_ += 0;

	const BinID bidstep = inputs_[0]->getStepoutStep();
	for ( int idx=0; idx<trcpos_.size(); idx++ )
	{
		const DataHolder* data = inputs_[0]->getData( relpos+trcpos_[idx]*bidstep, zintv );
		if ( !data ) return false;
		indata_.replace( idx, data );
	}
	
	indataidx_ = getDataIndex( 0 );
	
	return true;
}

bool ExternalAttrib::computeData( const DataHolder& output, const BinID& relpos, 
				int z0, int nrsamples, int threadid ) const
{
	if ( indata_.isEmpty() || output.isEmpty() )
		return false;
	
	const int nrtraces = indata_.size();
	proc_->resize( nrsamples );
	BinID bin = getCurrentPosition();
	for (int trcidx=0; trcidx<nrtraces; trcidx++)
	{
		int offset = trcidx*nrsamples;
		const DataHolder* data = indata_[trcidx];
		for ( int idx=0; idx<nrsamples; idx++ )
		{
			float val = getInputValue(*data, indataidx_, idx, z0);
			val = mIsUdf(val)?0.0f:val;
			proc_->setInput( 0, idx+offset, val );
		}
	}

	proc_->compute( z0, bin.inl(), bin.crl() );

	for ( int idx=0; idx<nrsamples; idx++ )
		for (int iout = 0; iout<proc_->numOutput(); iout++)
		{
			float val = proc_->getOutput(iout, idx);
			setOutputValue( output, iout, idx, z0, val );
		}
	return true;
}

}; //namespace

