/*Copyright (C) 2014 Wayne Mogg. All rights reserved.
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
 * 
-*/
#include "uirspecattrib.h"
#include "rspecattrib.h"

#include "attribdesc.h"
#include "attribparam.h"
#include "trckeyzsampling.h"
#include "survinfo.h"
#include "uiattribfactory.h"
#include "uiattrsel.h"
#include "uigeninput.h"
#include "uimsg.h"
#include "uispinbox.h"
#include "uistrings.h"
#include "uibutton.h"

#include "wmplugins.h"

using namespace Attrib;

mInitAttribUI(uiRSpecAttrib,RSpecAttrib,"Recursive spectral decomposition",wmPlugins::sKeyWMPlugins())


uiRSpecAttrib::uiRSpecAttrib( uiParent* p, bool is2d )
: uiAttrDescEd(p,is2d,HelpKey("wgm", "rspec"))
{
	inpfld_ = createInpFld( is2d );
	
	gatefld_ = new uiGenInput( this, gateLabel(), DoubleInpIntervalSpec().setName("Z start",0).setName("Z stop",1) );
    gatefld_->attach( alignedBelow, inpfld_ );
	
	uiString lbl;
	const bool zistime = SI().zDomain().isTime();
	const bool zismeter = SI().zDomain().isDepth() && !SI().depthsInFeet();
	if (zistime)
	    lbl = tr("Output Frequency (Hz)");
	else if (zismeter)
	    lbl = tr("Output Wavenumber (/km)");
	else
	    lbl = tr("Output Wavenumber (/kft)");

	freqfld_ = new uiLabeledSpinBox( this, lbl, 1 );
	freqfld_->attach( alignedBelow, gatefld_ );
	freqfld_->box()->doSnap( true );

	stepfld_ = new uiLabeledSpinBox( this, uiStrings::sStep(), 1 );
	stepfld_->attach( rightTo, freqfld_ );
	stepfld_->box()->valueChanged.notify(mCB(this,uiRSpecAttrib,stepChg));
    
    reassignbut_ = new uiCheckBox(this, tr("With Reassignment"));
    reassignbut_->attach(alignedBelow, freqfld_);
    reassignbut_->setChecked(false);
	
	stepChg(0);
	
	setHAlignObj( inpfld_ );
}

void uiRSpecAttrib::inputSel( CallBacker* )
{
	if ( !*inpfld_->getInput() ) return;
	
	TrcKeyZSampling cs;
	if ( !inpfld_->getRanges(cs) )
		cs.init(true);
	
	const float nyqfreq = 0.5f / cs.zsamp_.step;
	
	const float freqscale = zIsTime() ? 1.f : 1000.f;
	const float scalednyqfreq = nyqfreq * freqscale;
	stepfld_->box()->setInterval( (float)0.5, scalednyqfreq/2 );
	stepfld_->box()->setStep( (float)0.5, true );
	freqfld_->box()->setMinValue( stepfld_->box()->getFValue() );
	freqfld_->box()->setMaxValue( scalednyqfreq );
}

void uiRSpecAttrib::stepChg( CallBacker* )
{
	if ( mIsZero(stepfld_->box()->getFValue(),mDefEps) )
	{
		stepfld_->box()->setValue( 1 );
		stepfld_->box()->setStep( 1, true );
	}
	else
	{
		freqfld_->box()->setMinValue( stepfld_->box()->getFValue() );
		freqfld_->box()->setStep( stepfld_->box()->getFValue(), true );
	}
}

int uiRSpecAttrib::getOutputIdx( float outval ) const
{
	const float step = stepfld_->box()->getFValue();
	return mNINT32(outval/step)-1;
}


float uiRSpecAttrib::getOutputValue( int idx ) const
{
	const float step = stepfld_->box()->getFValue();
	return float((idx+1)*step);
}

bool uiRSpecAttrib::setParameters( const Attrib::Desc& desc )
{
	if ( desc.attribName() != RSpecAttrib::attribName() )
		return false;
	mIfGetFloatInterval(RSpecAttrib::gateStr(), gate, gatefld_->setValue(gate));

	const float freqscale = zIsTime() ? 1.f : 1000.f;
	mIfGetFloat(RSpecAttrib::stepStr(), step, stepfld_->box()->setValue(step*freqscale) );
    mIfGetBool(RSpecAttrib::reassignStr(), reassign, reassignbut_->setChecked(reassign));
	
	stepChg(0);
	return true;
}


bool uiRSpecAttrib::setInput( const Attrib::Desc& desc )
{
	putInp( inpfld_, desc, 0 );
	inputSel(0);
	return true;
}

bool uiRSpecAttrib::setOutput( const Desc& desc )
{
	const float freq = getOutputValue( desc.selectedOutput() );
	freqfld_->box()->setValue( freq );
	return true;
}

bool uiRSpecAttrib::getParameters( Attrib::Desc& desc )
{
	if ( desc.attribName() != RSpecAttrib::attribName() )
		return false;
	
	mSetFloatInterval( RSpecAttrib::gateStr(), gatefld_->getFInterval() );
	
	const float freqscale = zIsTime() ? 1.f : 1000.f;
	mSetFloat( RSpecAttrib::stepStr(), stepfld_->box()->getFValue()/freqscale );
    mSetBool(RSpecAttrib::reassignStr(), reassignbut_->isChecked());
	
	return true;
}


bool uiRSpecAttrib::getInput( Attrib::Desc& desc )
{
	fillInp( inpfld_, desc, 0 );
	return true;
}

bool uiRSpecAttrib::getOutput( Desc& desc )
{
	checkOutValSnapped();
	const int freqidx = getOutputIdx( freqfld_->box()->getFValue() );
	fillOutput( desc, freqidx );
	return true;
}

void uiRSpecAttrib::getEvalParams( TypeSet<EvalParam>& params ) const
{
	EvalParam ep( "Frequency" ); ep.evaloutput_ = true;
	params += ep;
	params += EvalParam( timegatestr(), RSpecAttrib::gateStr() );
}

void uiRSpecAttrib::checkOutValSnapped() const
{
	const float oldfreq = freqfld_->box()->getFValue();
	const int freqidx = getOutputIdx( oldfreq );
	const float freq = getOutputValue( freqidx );
	if ( oldfreq>0.5 && oldfreq!=freq )
	{
		uiString wmsg = tr( "Chosen output frequency \n"
							"does not fit with frequency step \n"
							"and will be snapped to nearest suitable frequency");
		uiMSG().warning( wmsg );
	}
}
