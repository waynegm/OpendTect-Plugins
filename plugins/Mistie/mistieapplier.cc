#include "mistieapplier.h"

#include "attribdataholder.h"
#include "attribdesc.h"
#include "attribdescset.h"
#include "attribfactory.h"
#include "attribparam.h"
#include "attribparamgroup.h"
#include "survinfo.h"
#include <math.h>
#include "arrayndimpl.h"
#include "errmsg.h"
#include "bufstring.h"
#include "survgeom2d.h"
#include "fourier.h"
#include "odcomplex.h"
#include "valseriesinterpol.h"
#include "hilberttransform.h"

namespace Attrib {

mAttrDefCreateInstance(MistieApplier)

void MistieApplier::initClass()
{
    mAttrStartInitClass

    StringParam* mistiefilepar = new StringParam( mistieFileStr() );
    desc->addParam( mistiefilepar );

    BoolParam* applyshift_ = new BoolParam( shiftStr() );
    applyshift_->setDefaultValue( true );
    desc->addParam( applyshift_ );

    BoolParam* applyphase_ = new BoolParam( phaseStr() );
    applyphase_->setDefaultValue( true );
    desc->addParam( applyphase_ );

    BoolParam* applyamp_ = new BoolParam( ampStr() );
    applyamp_->setDefaultValue( true );
    desc->addParam( applyamp_ );

    desc->addInput( InputSpec("Data",true) );

    desc->setNrOutputs( Seis::UnknowData, 1 );
    desc->setLocality(Desc::SingleTrace);

    mAttrEndInitClass
}

MistieApplier::MistieApplier( Desc& desc )
: Provider( desc )
{
    if ( !isOK() ) return;

    mGetString( mistiefile_, mistieFileStr() );
    mGetBool( applyshift_, shiftStr() );
    mGetBool( applyphase_, phaseStr() );
    mGetBool( applyamp_, ampStr() );

    zmargin_ = Interval<int>( -30, 30 );

    if (!corrections_.read( mistiefile_ )) {
        ErrMsg("MistieApplier::MistieApplier - error reading mistie correction file");
    }
}

bool MistieApplier::getInputData( const BinID& relpos, int zintv )
{
    data_ = inputs_[0]->getData( relpos, zintv );

    dataidx_ = getDataIndex(0);

    return data_;
}

void MistieApplier::prepareForComputeData()
{
    BufferString dname;
    if (is2D()) {
        Pos::GeomID geomid = getGeomID();
        mDynamicCastGet( const Survey::Geometry2D*, geom2d, Survey::GM().getGeometry(geomid) );
        dname = geom2d->getName();
    } else {
        dname = "3D_"; dname += getDesc().getInput(0)->userRef();
    }
    if (!corrections_.get(dname, shift_, phase_, amp_)) {
        shift_ = 0.0;
        phase_ = 0.0;
        amp_ = 1.0;
        BufferString tmp("MistieApplier::prepareForComputeData - no mistie corrections for: ");
        tmp += dname;
        ErrMsg(tmp);
    }
}

bool MistieApplier::computeData( const DataHolder& output, const BinID& relpos, int z0, int nrsamples, int threadid) const
{
    if ( !data_ )
        return false;

    const int sz = zmargin_.width() + nrsamples;

    Array1DImpl<float> data(sz);
    if (applyshift_ && !mIsZero(shift_,mDefEps)) {
        ValueSeries<float>* dataseries = data_->series(dataidx_);
        ValueSeriesInterpolator<float> interp( data_->nrsamples_-1 );
        int offs = z0 - data_->z0_;
        float shift = shift_/(refstep_* SI().zDomain().userFactor());
        for (int idx=0; idx<sz; idx++) {
            float pos = zmargin_.start_ + idx + offs - shift;
            data.set(idx, interp.value( *dataseries, pos ));
        }
    } else {
        for (int idx=0; idx<sz; idx++)
            data.set(idx, getInputValue( *data_, dataidx_, zmargin_.start_+idx, z0 ));
    }

    if (applyphase_ && mIsEqual(phase_, 180.0, mDefEps)) {
        for (int idx=0; idx<sz; idx++) {
            float val = data.get(idx);
            data.set(idx, mIsUdf(val) ? val : -val);
        }
    } else if (applyphase_ && !mIsZero(phase_, mDefEps)) {
        HilbertTransform ht;
        ht.setInputInfo(Array1DInfoImpl(sz));
        ht.setHalfLen( zmargin_.width()/2 );
        ht.setCalcRange(0,0);
        ht.setDir(true);
        if (!ht.init())
            return false;
        Array1DImpl<float> idata(sz);
        ht.transform( data, idata );
        float rfact = cos(Math::toRadians(phase_));
        float ifact = sin(Math::toRadians(phase_));
        for (int idx=0; idx<sz; idx++) {
            float val = data.get(idx);
            float ival = idata.get(idx);
            data.set(idx, mIsUdf(val)||mIsUdf(ival) ? mUdf(float) : val*rfact-ival*ifact);
        }
    }

    if (applyamp_ && !mIsEqual(amp_, 1.0, mDefEps)) {
        for (int idx=0; idx<sz; idx++) {
            float val = data.get(idx);
            data.set(idx, mIsUdf(val) ? val : val*amp_);
        }
    }

    for (int idx=0; idx<nrsamples; idx++)
        setOutputValue( output, 0, idx, z0, data.get(idx-zmargin_.start_) );

    return true;
}

}






