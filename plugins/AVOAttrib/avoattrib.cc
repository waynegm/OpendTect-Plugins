/*Copyright (C) 2014 Wayne Mogg All rights reserved.
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
 * Date:          February 2014
 * ________________________________________________________________________
 * 
 * -*/


#include "avoattrib.h"
#include "attribdataholder.h"
#include "attribdesc.h"
#include "attribfactory.h"
#include "attribparam.h"

#include "math2.h"


namespace Attrib
{

mAttrDefCreateInstance(AVOAttrib)    
    
void AVOAttrib::initClass()
{
	mAttrStartInitClassWithUpdate

    EnumParam* output = new EnumParam( outputStr() );
    output->addEnum( "Fluid Factor" );
    output->addEnum( "Lithology Factor" );
	output->addEnum( "Porosity Factor" );
	output->addEnum( "Crossplot Angle" );
	output->addEnum( "Crossplot Deviation" );
	output->addEnum( "AVO Class" );
	desc->addParam( output );
    
    FloatParam* slope = new FloatParam( slopeStr() );
    slope->setDefaultValue( 1 );
    desc->addParam( slope );

    FloatParam* intercept_dev = new FloatParam( intercept_devStr() );
    desc->addParam( intercept_dev );
    intercept_dev->setDefaultValue( 0 );

	FloatParam* gradient_dev = new FloatParam( gradient_devStr() );
	desc->addParam( gradient_dev );
	gradient_dev->setDefaultValue( 0 );

	FloatParam* correlation = new FloatParam( correlationStr() );
	desc->addParam( correlation );
	correlation->setDefaultValue( 0 );
	
	FloatParam* class2width = new FloatParam( class2Str() );
	desc->addParam( class2width );
	correlation->setDefaultValue( 0 );

	desc->addOutputDataType( Seis::UnknowData );
	desc->addInput( InputSpec("Intercept cube",true) );
	desc->addInput( InputSpec("Gradient cube",true) );

    mAttrEndInitClass
}

void AVOAttrib::updateDesc( Desc& desc )
{
	BufferString output = desc.getValParam( outputStr() )->getStringValue();
	
	desc.setParamEnabled( intercept_devStr(), output=="Crossplot Angle" || output=="Crossplot Deviation" );
	desc.setParamEnabled( gradient_devStr(), output=="Crossplot Angle" || output=="Crossplot Deviation" );
	desc.setParamEnabled( correlationStr(), output=="Crossplot Deviation" );
	desc.setParamEnabled( class2Str(), output=="AVO Class" );
}

AVOAttrib::AVOAttrib( Desc& desc )
    : Provider( desc )
{
    if ( !isOK() ) return;

    mGetEnum( output_, outputStr() );
    mGetFloat( slope_, slopeStr() );
    mGetFloat( intercept_dev_, intercept_devStr() );
	mGetFloat( gradient_dev_, gradient_devStr() );
	mGetFloat( correlation_, correlationStr() );
	mGetFloat( class2width_, class2Str() );
	
	xplotang_ = slope_ < 0 ? M_PI_2 + atan(slope_) : M_PI_2 - atan(slope_);

}


bool AVOAttrib::getInputData( const BinID& relpos, int zintv )
{
	interceptdata_ = inputs_[0]->getData( relpos, zintv );
	gradientdata_ = inputs_[1]->getData( relpos, zintv );
	if ( !interceptdata_ || !gradientdata_ )
		return false;

	interceptdataidx_ = getDataIndex( 0 );
	gradientdataidx_ = getDataIndex( 1 );
	
    return true;
}


bool AVOAttrib::computeData( const DataHolder& output, const BinID& relpos,
			   int z0, int nrsamples, int threadid ) const
{
	float	idev2 = intercept_dev_*intercept_dev_;
	float	gdev2 = gradient_dev_*gradient_dev_;
	float	igdev = intercept_dev_*gradient_dev_;
    for ( int idx=0; idx<nrsamples; idx++ ) {
		const float intercept = getInputValue( interceptdata_[0], interceptdataidx_, idx, z0);
		const float gradient = getInputValue( gradientdata_[0], gradientdataidx_, idx, z0);
		const float slint = slope_*intercept;
		
		float outval = mUdf(float);
		
		if (!mIsUdf(intercept) && !mIsUdf(gradient)) {
			if (output_ == FluidFactor) 
				outval = intercept*cos(xplotang_) + gradient*sin(xplotang_);
			else if (output_ == LithFactor)
				outval = -intercept*sin(xplotang_) + gradient*cos(xplotang_);
			else if (output_ == PorosityFactor) {
				if (mIsEqual(gradient, slint, mDefEpsF)) 
					outval = 0.0;
				else if (gradient > slint)
					outval = intercept*sin(xplotang_) - gradient*cos(xplotang_);
				else
					outval = -intercept*sin(xplotang_) + gradient*cos(xplotang_);
			} else if (output_ == CrossplotAngle) {
				outval = Math::toDegrees(Math::ACos((-intercept*sin(xplotang_) + gradient*cos(xplotang_))/sqrt(intercept*intercept+gradient*gradient)));
				outval = gradient > slint ? outval-180: outval;
			} else if (output_ == CrossplotDeviation) {
				outval = sqrt(intercept*intercept/idev2+gradient*gradient/gdev2-2*correlation_*intercept*gradient/igdev);
			} else if (output_ == AVOClass) {
				if (intercept<=class2width_ && intercept>=-class2width_) {
					outval = gradient > slint ? -2: 2;
				} else if (gradient<=slint && gradient>=0 && intercept<=0)
					outval = 4;
				else if (gradient>slint && gradient<=0 && intercept>=0)
					outval = -4;
				else if (gradient<=slint && gradient<0 && intercept<=0)
					outval = 3;
				else if (gradient>slint && gradient>=0 && intercept>=0)
					outval = -3;
				else if (gradient<=slint && gradient<0 && intercept>=0)
					outval = 1;
				else if (gradient>slint && gradient>0 && intercept<=0)
					outval = -1;
			}
		}
		
		setOutputValue( output, 0, idx, z0, outval );
    }

    return true;
}


} // namespace Attrib

