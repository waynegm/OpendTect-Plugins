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
#include "envvars.h"
#include "filepath.h"
#include "oddirs.h"
#include "pythonaccess.h"
#include "settings.h"
#include "survinfo.h"

#include "extproc.h"

FilePath getPythonPath()
{
    FilePath fp;

    BufferString pythonstr( sKey::Python() ); pythonstr.toLower();
    const IOPar& pythonsetts = Settings::fetch( pythonstr );
    OD::PythonSource source;
    if (!OD::PythonSourceDef().parse(pythonsetts,OD::PythonAccess::sKeyPythonSrc(),source))
	source = OD::System;


    if ( source == OD::Custom )
    {
	BufferString virtenvloc, virtenvnm;
	pythonsetts.get(OD::PythonAccess::sKeyEnviron(),virtenvloc);
	pythonsetts.get(sKey::Name(),virtenvnm);
	fp = FilePath("/", virtenvloc, "envs", virtenvnm, "bin" );
    }

    fp.add( pythonstr );

    return fp;
}

namespace Attrib
{

mAttrDefCreateInstance(ExternalAttrib)    

ExtProc* ExternalAttrib::dProc_ = NULL;
BufferString ExternalAttrib::exdir_ = "";

void ExternalAttrib::initClass()
{
	mAttrStartInitClassWithUpdate

	StringParam* interpfilepar = new StringParam( interpFileStr() );
	interpfilepar->setDefaultValue( getPythonPath().fullPath() );
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

	IntParam* selection = new IntParam( selectStr() );
    desc->addParam( selection );
	selection->setDefaultValue(0);

	for (int i=0; i<cNrParams; i++) {
		FloatParam* par = new FloatParam( BufferString(parStr()).add(i) );
		desc->addParam( par );
		par->setDefaultValue(0);
	}
	
	desc->addInput(InputSpec("Input data", true));
	desc->addOutputDataType( Seis::UnknowData );
	
	if ( GetEnvVarYN("OD_EX_DIR") )
	    exdir_ = GetEnvVar("OD_EX_DIR");
	else {
	    FilePath fp( GetScriptDir(),"python","wmpy" );
	    if ( fp.exists() )
		exdir_ = fp.fullPath();
	    else if ( GetEnvVarYN("OD_USER_PLUGIN_DIR") ) {
		FilePath ufp( GetEnvVar("OD_USER_PLUGIN_DIR"), "bin", "python", "wmpy" );
		if ( ufp.exists() )
		    exdir_ = ufp.fullPath();
	    }
	}

	
	mAttrEndInitClass
}

void ExternalAttrib::updateDesc( Desc& desc )
{

	BufferString iname = desc.getValParam( interpFileStr() )->getStringValue();
	BufferString fname = desc.getValParam( exFileStr() )->getStringValue();
	fname.replace( "%OD_EX_DIR%", ExternalAttrib::exdir_ );
#ifdef __win__
	fname.replace("/","\\");
#endif
	
	if (dProc_) {
        if (dProc_->getFile() != fname) {
            delete dProc_;
            dProc_ = new ExtProc(fname.str(), iname.str());
		}
	} else {
        dProc_ = new ExtProc(fname.str(), iname.str());
	}

	desc.setParamEnabled(zmarginStr(), dProc_->hasZMargin());
	desc.setParamEnabled(stepoutStr(), dProc_->hasStepOut());
	desc.setParamEnabled(selectStr(), dProc_->hasSelect());
	for (int i=0; i<cNrParams; i++) {
		desc.setParamEnabled(BufferString(parStr()).add(i), dProc_->hasParam(i));
	}
	
	if (dProc_->hasOutput())
		desc.setNrOutputs(Seis::UnknowData, dProc_->numOutput());
	else
		desc.setNrOutputs(Seis::UnknowData, 1);

    if ( desc.nrInputs() != dProc_->numInput() ) {
		while ( desc.nrInputs() )
			desc.removeInput(0);
		for ( int idx=0; idx<dProc_->numInput(); idx++ ) {
			BufferString bfs = "Input "; bfs += idx+1;
			desc.addInput( InputSpec(bfs, true) );
		}
    }
	
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
	
	mGetString( interpfile_, interpFileStr() );
	mGetString( exfile_, exFileStr() );
	exfile_.replace( "%OD_EX_DIR%", ExternalAttrib::exdir_ );
#ifdef __win__
	exfile_.replace("/","\\");
#endif
	
	proc_= new ExtProc( exfile_.str(), interpfile_.str() );
	
	if (proc_) {
		if (proc_->hasZMargin()) {
			mGetIntInterval( zmargin_, zmarginStr() );
			proc_->setZMargin(zmargin_);
		}
		if (proc_->hasStepOut()) {
			mGetBinID( stepout_, stepoutStr() );
			proc_->setStepOut(stepout_);
		} else {
			stepout_ = BinID(0,0);
		}
		if (proc_->hasSelect()) {
            mGetInt( selection_, selectStr());
            proc_->setSelection( selection_);
        }
        par_.setSize(cNrParams);
		for (int i=0; i<cNrParams; i++) {
			if (proc_->hasParam(i)) {
				BufferString s(parStr());
				s.add(i);
				mGetFloat(par_[i], s);
				proc_->setParam(i, par_[i]);
			}
		}
		nrout_ = proc_->numOutput();
		nrin_ = proc_->numInput();
		getTrcPos();
		int ninl = stepout_.inl()*2 + 1;
		int ncrl = stepout_.crl()*2 + 1;
		proc_->setSeisInfo( ninl, ncrl, inlDist()*SI().inlStep(), crlDist()*SI().crlStep(), zFactor(), dipFactor() );
	} else 
		ErrMsg("ExternalAttrib::ExternalAttrib - error creating extrenal procedure");
}

ExternalAttrib::~ExternalAttrib()
{
	if (proc_)
		delete proc_;
}

bool ExternalAttrib::allowParallelComputation() const
{
	return proc_->doParallel(); }

bool ExternalAttrib::getTrcPos()
{
	trcpos_.erase();
    int trcidx = 0;
    centertrcidx_ = 0;
	BinID bid;
	for ( bid.inl()=-stepout_.inl(); bid.inl()<=stepout_.inl(); bid.inl()++ ) {
		for ( bid.crl()=-stepout_.crl(); bid.crl()<=stepout_.crl(); bid.crl()++ ) {
      	    if ( !bid.inl() && !bid.crl() )
                centertrcidx_ = trcidx;
			trcpos_ += bid;
            trcidx++;
        }
    }
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
	while ( indata_.size() < trcpos_.size()*nrin_ ) {
		indata_ += 0;
	}
	while (indataidx_.size() < nrin_) {
		indataidx_.add(-1);
	}

	const BinID bidstep = inputs_[0]->getStepoutStep();
	for (int iin=0; iin<nrin_; iin++) {
		for ( int idx=0; idx<trcpos_.size(); idx++ )
		{
			BinID pos = relpos + trcpos_[idx] * bidstep;
			const DataHolder* data = inputs_[iin]->getData( pos, zintv );
			if ( !data ) {
                pos = relpos + trcpos_[centertrcidx_]*bidstep;
                data = inputs_[iin]->getData( pos, zintv );
                if ( !data ) return false;
			}
			indata_.replace( iin*trcpos_.size()+idx, data );
		}
		indataidx_[iin] = getDataIndex(iin);
	}
	
	return true;
}

bool ExternalAttrib::computeData( const DataHolder& output, const BinID& relpos, int z0, int nrsamples, int threadid ) const
{
	if ( indata_.isEmpty() || output.isEmpty() )
		return false;
	if (proc_->isOK()) {
		const int nrtraces = trcpos_.size();
		const int sz = zmargin_.width() + nrsamples;
		ProcInst* pi = proc_->getIdleInst( sz );
		BinID bin = getCurrentPosition();
		for (int iin = 0; iin<nrin_; iin++) {
			for (int trcidx=0; trcidx<nrtraces; trcidx++)
			{
				const DataHolder* data = indata_[iin*nrtraces+trcidx];
				for ( int idx=0; idx<sz; idx++ )
				{
					float val = getInputValue(*data, indataidx_[iin], zmargin_.start+idx, z0);
					val = mIsUdf(val)?0.0f:val;
					proc_->setInput( pi, iin, trcidx, idx, val );
				}
			}
		}

		proc_->compute( pi, z0, bin.inl(), bin.crl() );
		for (int iout = 0; iout<nrout_; iout++) {
			if (outputinterest_[iout]) {
				for ( int idx=0; idx<nrsamples; idx++ ) {
					float val = proc_->getOutput( pi, iout, idx-zmargin_.start);
					setOutputValue( output, iout, idx, z0, val );
				}
			}
		}
		proc_->setInstIdle( pi );
		return true;
	} else
		return false;
}

const Interval<int>*	ExternalAttrib::desZSampMargin(int,int) const
{ 
	if (zmargin_ == Interval<int>(0,0))
		return 0;
	else
		return &zmargin_; 
}

const BinID* ExternalAttrib::desStepout(int input,int output) const
{ 
	if (stepout_ == BinID(0,0))
		return 0;
	else
		return &stepout_;
}


}; //namespace

