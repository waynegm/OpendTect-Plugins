/*
 *   uiEFDAttrib Plugin -  Empirical Fourier Spectral Decomposition
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

#include "uiefdspectrumattrib.h"
#include "efdspectrumattrib.h"

#include "attribdesc.h"
#include "attribparam.h"
#include "survinfo.h"
#include "uiattribfactory.h"
#include "uiattrsel.h"
#include "uigeninput.h"
#include "uibutton.h"
#include "uispinbox.h"
#include "trckeyzsampling.h"
#include "uimsg.h"

#include "wmplugins.h"

using namespace Attrib;

mInitAttribUI(uiEFDSpectrumAttrib,EFDSpectrumAttrib,"EFD Spectral Decomposition",wmPlugins::sKeyWMPlugins())


uiEFDSpectrumAttrib::uiEFDSpectrumAttrib( uiParent* p, bool is2d )
: uiAttrDescEd(p,is2d,HelpKey("wgm", "efd"))
, TestPanelAdaptor()
{
    inpfld_ = createInpFld( is2d );

    modesfld_ = new uiGenInput( this, tr("Number of Modes"), IntInpSpec(5, 1, 20));
    modesfld_->attach( alignedBelow, inpfld_ );

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
    freqfld_->attach( alignedBelow, modesfld_ );
    freqfld_->box()->doSnap( true );

    stepfld_ = new uiLabeledSpinBox( this, uiStrings::sStep(), 1 );
    stepfld_->attach( rightTo, freqfld_ );
    stepfld_->box()->valueChanged.notify(mCB(this,uiEFDSpectrumAttrib,stepChg));

    uiString tfstr = tr("Display Time/Frequency panel");
    tfpanelbut_ = new uiPushButton( this, tfstr, mCB(this, uiEFDSpectrumAttrib, showPosDlgCB), true );
    tfpanelbut_->attach( alignedBelow, freqfld_ );

    stepChg(0);
    setHAlignObj( inpfld_ );
}

void uiEFDSpectrumAttrib::inputSel( CallBacker* )
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

void uiEFDSpectrumAttrib::stepChg( CallBacker* )
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

int uiEFDSpectrumAttrib::getOutputIdx( float outval ) const
{
    const float step = stepfld_->box()->getFValue();
    return mNINT32(outval/step)-1;
}

float uiEFDSpectrumAttrib::getOutputValue( int idx ) const
{
    const float step = stepfld_->box()->getFValue();
    return float((idx+1)*step);
}

bool uiEFDSpectrumAttrib::setParameters( const Attrib::Desc& desc )
{
    if ( desc.attribName() != EFDSpectrumAttrib::attribName() )
	return false;

    mIfGetInt(EFDSpectrumAttrib::nrmodesStr(), nrmodes, modesfld_->setValue(nrmodes));
    const float freqscale = zIsTime() ? 1.f : 1000.f;
    mIfGetFloat(EFDSpectrumAttrib::stepStr(), step, stepfld_->box()->setValue(step*freqscale) );

    stepChg(0);
    return true;
}


bool uiEFDSpectrumAttrib::setInput( const Attrib::Desc& desc )
{
    putInp( inpfld_, desc, 0 );
    inputSel(0);
    return true;
}

bool uiEFDSpectrumAttrib::setOutput( const Desc& desc )
{
    const float freq = getOutputValue( desc.selectedOutput() );
    freqfld_->box()->setValue( freq );
    return true;
}

bool uiEFDSpectrumAttrib::getParameters( Attrib::Desc& desc )
{
    if ( desc.attribName() != EFDSpectrumAttrib::attribName() )
	return false;

    mSetInt( EFDSpectrumAttrib::nrmodesStr(), modesfld_->getIntValue() );
    const float freqscale = zIsTime() ? 1.f : 1000.f;
    mSetFloat( EFDSpectrumAttrib::stepStr(), stepfld_->box()->getFValue()/freqscale );
    return true;
}


bool uiEFDSpectrumAttrib::getInput( Attrib::Desc& desc )
{
    inpfld_->processInput();
    fillInp( inpfld_, desc, 0 );
    return true;
}

bool uiEFDSpectrumAttrib::getOutput( Desc& desc )
{
    checkOutValSnapped();
    const int freqidx = getOutputIdx( freqfld_->box()->getFValue() );
    fillOutput( desc, freqidx );
    return true;
}

void uiEFDSpectrumAttrib::getEvalParams( TypeSet<EvalParam>& params ) const
{
    EvalParam ep( "Frequency" ); ep.evaloutput_ = true;
    params += ep;
    params += EvalParam( "Number of Modes", EFDSpectrumAttrib::nrmodesStr() );
}

void uiEFDSpectrumAttrib::checkOutValSnapped() const
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

void uiEFDSpectrumAttrib::showPosDlgCB( CallBacker* )
{
    if ( inpfld_->attribID() == DescID::undef() )
    {
	uiMSG().error( tr("Please fill in the Input Data field") );
	return;
    }
    delete( testpanel_ );
    testpanel_ = new uiAttribTestPanel<uiEFDSpectrumAttrib>( *this,
				     "Compute all frequencies for a single trace",
				     "Spectral Decomposition time/frequency spectrum",
				     "Time Frequency spectrum" );

    testpanel_->showPosDlg();
}

#define mSetParam( type, nm, str, fn )\
{ \
    mDynamicCastGet(type##Param*, nm, desc->getValParam(str))\
    nm->setValue( fn );\
}

void uiEFDSpectrumAttrib::fillTestParams( Attrib::Desc* desc ) const
{
    mSetParam(Int,nmodes, EFDSpectrumAttrib::nrmodesStr(), modesfld_->getIntValue())

    //show Frequencies with a step of 1 in Time and 1e-3 in Depth,
    //independently of what the user can have specified previously
    //in the output/step fields
    //little trick to have correct axes annotation (at least in time)
    mSetParam(Float,dfreq,EFDSpectrumAttrib::stepStr(), zIsTime() ? 1.f : 0.001f )
}
