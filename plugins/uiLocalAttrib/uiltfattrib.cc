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

using namespace Attrib;

mInitAttribUI(uiLTFAttrib,LTFAttrib,"Spectral Decomposition by Local Attribute",sKeyFreqGrp())


uiLTFAttrib::uiLTFAttrib( uiParent* p, bool is2d )
: uiAttrDescEd(p,is2d,HelpKey("wgm", "ltf"))

{
	inpfld_ = createInpFld( is2d );
	
	BufferString lbl( "Output frequency (" );
	lbl += zIsTime() ? "Hz" :
	(SI().zInMeter() ? "cycles/km" : "cycles/kft"); lbl += ")";
	freqfld_ = new uiLabeledSpinBox( this, lbl, 1 );
	freqfld_->attach( alignedBelow, inpfld_ );
	freqfld_->box()->doSnap( true );
	
	smoothfld_ = new uiLabeledSpinBox( this, tr("Smoothing Radius (samples)") );
	smoothfld_->box()->setMinValue( 2 );
	smoothfld_->box()->setStep( 1, true );
	smoothfld_->attach( alignedBelow, freqfld_ );
	
	niterfld_ = new uiLabeledSpinBox( this, tr("Iterations") );
	niterfld_->box()->setMinValue( 50 );
	niterfld_->box()->setStep( 10, true );
	niterfld_->attach( alignedBelow, smoothfld_ );
	
	marginfld_ = new uiLabeledSpinBox( this, tr("Margin (samples)") );
	marginfld_->box()->setMinValue( 0 );
	marginfld_->box()->setStep( 1, true );
	marginfld_->attach( alignedBelow, niterfld_ );
	
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
	freqfld_->box()->setMinValue( 0.0f );
	freqfld_->box()->setMaxValue( scalednyqfreq );
}


bool uiLTFAttrib::setParameters( const Attrib::Desc& desc )
{
	if ( desc.attribName() != LTFAttrib::attribName() )
		return false;
	
	mIfGetFloat(LTFAttrib::freqStr(),freq, freqfld_->box()->setValue(freq) )
	mIfGetInt(LTFAttrib::smoothStr(),smooth, smoothfld_->box()->setValue(smooth) )
	mIfGetInt(LTFAttrib::niterStr(),niter, niterfld_->box()->setValue(niter) )
	mIfGetInt(LTFAttrib::marginStr(),margin, marginfld_->box()->setValue(margin) )

	return true;
}


bool uiLTFAttrib::setInput( const Attrib::Desc& desc )
{
	putInp( inpfld_, desc, 0 );
	return true;
}


bool uiLTFAttrib::getParameters( Attrib::Desc& desc )
{
	if ( desc.attribName() != LTFAttrib::attribName() )
		return false;
	
	mSetFloat( LTFAttrib::freqStr(), freqfld_->box()->getValue() );
	mSetInt( LTFAttrib::smoothStr(), smoothfld_->box()->getValue() );
	mSetInt( LTFAttrib::niterStr(), niterfld_->box()->getValue() );
	mSetInt( LTFAttrib::marginStr(), marginfld_->box()->getValue() );
	
	return true;
}


bool uiLTFAttrib::getInput( Attrib::Desc& desc )
{
	inpfld_->processInput();
	fillInp( inpfld_, desc, 0 );
	return true;
}


