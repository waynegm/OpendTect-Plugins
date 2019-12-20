/*
 *   uiLocalAttrib Plugin
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

#include "uiltfattrib.h"
#include "ltfattrib.h"

#include "attribdesc.h"
#include "attribparam.h"
#include "survinfo.h"
#include "uiattribfactory.h"
#include "uiattrsel.h"
#include "uigeninput.h"
#include "uispinbox.h"
#include "trckeyzsampling.h"
#include "uimsg.h"

#include "wmplugins.h"

using namespace Attrib;

mInitAttribUI(uiLTFAttrib,LTFAttrib,"Spectral Decomposition by Local Attribute",wmPlugins::sKeyWMPlugins())


uiLTFAttrib::uiLTFAttrib( uiParent* p, bool is2d )
: uiAttrDescEd(p,is2d,HelpKey("wgm", "ltf"))

{
	inpfld_ = createInpFld( is2d );

    gatefld_ = new uiGenInput( this, gateLabel(), DoubleInpIntervalSpec().setName("Z start",0).setName("Z stop",1) );
    gatefld_->attach( alignedBelow, inpfld_ );
    
	BufferString lbl( "Output frequency (" );
	lbl += zIsTime() ? "Hz" :
	(SI().zInMeter() ? "cycles/km" : "cycles/kft"); lbl += ")";
	freqfld_ = new uiLabeledSpinBox( this, lbl, 1 );
	freqfld_->attach( alignedBelow, gatefld_ );
	freqfld_->box()->doSnap( true );

    stepfld_ = new uiLabeledSpinBox( this, uiStrings::sStep(), 1 );
    stepfld_->attach( rightTo, freqfld_ );
    stepfld_->box()->valueChanged.notify(mCB(this,uiLTFAttrib,stepChg));
    
    stepChg(0);
    
//	smoothfld_ = new uiLabeledSpinBox( this, tr("Smoothing Radius (samples)") );
//	smoothfld_->box()->setMinValue( 2 );
//	smoothfld_->box()->setStep( 1, true );
//	smoothfld_->attach( alignedBelow, freqfld_ );
	
	niterfld_ = new uiLabeledSpinBox( this, tr("Iterations") );
	niterfld_->box()->setMinValue( 50 );
	niterfld_->box()->setStep( 10, true );
	niterfld_->attach( alignedBelow, freqfld_ );
	
//	marginfld_ = new uiLabeledSpinBox( this, tr("Margin (samples)") );
//	marginfld_->box()->setMinValue( 0 );
//	marginfld_->box()->setStep( 1, true );
//	marginfld_->attach( alignedBelow, niterfld_ );
	
	setHAlignObj( inpfld_ );
}

void uiLTFAttrib::inputSel( CallBacker* )
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

void uiLTFAttrib::stepChg( CallBacker* )
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

int uiLTFAttrib::getOutputIdx( float outval ) const
{
    const float step = stepfld_->box()->getFValue();
    return mNINT32(outval/step)-1;
}

float uiLTFAttrib::getOutputValue( int idx ) const
{
    const float step = stepfld_->box()->getFValue();
    return float((idx+1)*step);
}

bool uiLTFAttrib::setParameters( const Attrib::Desc& desc )
{
	if ( desc.attribName() != LTFAttrib::attribName() )
		return false;
	
    mIfGetFloatInterval(LTFAttrib::gateStr(), gate, gatefld_->setValue(gate));
    const float freqscale = zIsTime() ? 1.f : 1000.f;
    mIfGetFloat(LTFAttrib::stepStr(), step, stepfld_->box()->setValue(step*freqscale) );
//    mIfGetFloat(LTFAttrib::freqStr(),freq, freqfld_->box()->setValue(freq) )
//	mIfGetInt(LTFAttrib::smoothStr(),smooth, smoothfld_->box()->setValue(smooth) )
	mIfGetInt(LTFAttrib::niterStr(),niter, niterfld_->box()->setValue(niter) )
//	mIfGetInt(LTFAttrib::marginStr(),margin, marginfld_->box()->setValue(margin) )

    stepChg(0);
	return true;
}


bool uiLTFAttrib::setInput( const Attrib::Desc& desc )
{
	putInp( inpfld_, desc, 0 );
    inputSel(0);
	return true;
}

bool uiLTFAttrib::setOutput( const Desc& desc )
{
    const float freq = getOutputValue( desc.selectedOutput() );
    freqfld_->box()->setValue( freq );
    return true;
}

bool uiLTFAttrib::getParameters( Attrib::Desc& desc )
{
	if ( desc.attribName() != LTFAttrib::attribName() )
		return false;
	
    mSetFloatInterval( LTFAttrib::gateStr(), gatefld_->getFInterval() );
    const float freqscale = zIsTime() ? 1.f : 1000.f;
    mSetFloat( LTFAttrib::stepStr(), stepfld_->box()->getFValue()/freqscale );
    
//    mSetFloat( LTFAttrib::freqStr(), freqfld_->box()->getValue() );
//	mSetInt( LTFAttrib::smoothStr(), smoothfld_->box()->getValue() );
	mSetInt( LTFAttrib::niterStr(), niterfld_->box()->getValue() );
//	mSetInt( LTFAttrib::marginStr(), marginfld_->box()->getValue() );
	
	return true;
}


bool uiLTFAttrib::getInput( Attrib::Desc& desc )
{
	inpfld_->processInput();
	fillInp( inpfld_, desc, 0 );
	return true;
}

bool uiLTFAttrib::getOutput( Desc& desc )
{
    checkOutValSnapped();
    const int freqidx = getOutputIdx( freqfld_->box()->getFValue() );
    fillOutput( desc, freqidx );
    return true;
}

void uiLTFAttrib::getEvalParams( TypeSet<EvalParam>& params ) const
{
    EvalParam ep( "Frequency" ); ep.evaloutput_ = true;
    params += ep;
    params += EvalParam( timegatestr(), LTFAttrib::gateStr() );
}

void uiLTFAttrib::checkOutValSnapped() const
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
