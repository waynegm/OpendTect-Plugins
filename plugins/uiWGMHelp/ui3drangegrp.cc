#include "ui3drangegrp.h"

#include "uigeninput.h"
#include "uigroup.h"
#include "uimsg.h"

WMLib::ui3DRangeGrp::ui3DRangeGrp( uiParent* p, const uiString& caption, bool snapToStep )
    : uiDlgGroup(p, caption)
    , stepSnap_(snapToStep)
{
    const Interval<int> startstoprg( -mUdf(int), mUdf(int) );
    const Interval<int> steprg( 1, mUdf(int) );
    IntInpIntervalSpec iis( true );
    iis.setLimits( startstoprg, -1 ).setLimits( steprg, 2 );
    iis.setName("Inl Start",0).setName("Inl Stop",1).setName("Inl step",2);
    inlfld_ = new uiGenInput(this, uiStrings::phrInline(uiStrings::sRange()), iis);
    inlfld_->valuechanged.notify( mCB(this,ui3DRangeGrp,rangeChg) );
    
    iis.setName("Crl Start",0).setName("Crl Stop",1).setName("Crl step",2);
    crlfld_ = new uiGenInput( this, tr("Cross-line range"), iis );
    crlfld_->attach(alignedBelow, inlfld_);
    crlfld_->valuechanged.notify( mCB(this,ui3DRangeGrp,rangeChg) );
    
    setHAlignObj( inlfld_ );
}
WMLib::ui3DRangeGrp::~ui3DRangeGrp()
{
    detachAllNotifiers();
}

bool WMLib::ui3DRangeGrp::fillPar( IOPar& par ) const
{
    par.set(sKey::Type(), sKey::Range());
    TrcKeySampling hs = getTrcKeySampling();
    hs.fillPar(par);
    return true;
}

TrcKeySampling  WMLib::ui3DRangeGrp::getTrcKeySampling() const
{
    TrcKeySampling hs;
    hs.start_.inl() = inlfld_->getIStepInterval().start;
    hs.stop_.inl() =  inlfld_->getIStepInterval().stop;
    hs.step_.inl() =  inlfld_->getIStepInterval().step;
    hs.start_.crl() = crlfld_->getIStepInterval().start;
    hs.stop_.crl() =  crlfld_->getIStepInterval().stop;
    hs.step_.crl() =  crlfld_->getIStepInterval().step;
    
    return hs;
}

void WMLib::ui3DRangeGrp::setTrcKeySampling(const TrcKeySampling& hs)
{
    inlfld_->setValue(hs.inlRange());
    crlfld_->setValue(hs.crlRange());
    rangeChg(0);
}

void WMLib::ui3DRangeGrp::displayFields(bool yn)
{
    inlfld_->displayField(yn);
    crlfld_->displayField(yn);
}

void WMLib::ui3DRangeGrp::displayFields(bool ranges, bool steps)
{
    inlfld_->displayField(ranges, 0, 0);
    inlfld_->displayField(ranges, 1, 0);
    inlfld_->displayField(steps, 2, 0);
    crlfld_->displayField(ranges, 0, 0);
    crlfld_->displayField(ranges, 1, 0);
    crlfld_->displayField(steps, 2, 0);
}

void WMLib::ui3DRangeGrp::setSensitive(bool yn)
{
    inlfld_->setSensitive(yn);
    crlfld_->setSensitive(yn);
}

void WMLib::ui3DRangeGrp::setSensitive(bool ranges, bool steps)
{
    inlfld_->setSensitive(ranges, 0, 0);
    inlfld_->setSensitive(ranges, 1, 0);
    inlfld_->setSensitive(steps, 2, 0);
    crlfld_->setSensitive(ranges, 0, 0);
    crlfld_->setSensitive(ranges, 1, 0);
    crlfld_->setSensitive(steps, 2, 0);
}

void WMLib::ui3DRangeGrp::rangeChg(Callbacker* cb)
{
    if (stepSnap_) {
        TrcKeySampling hs;
        int step = inlfld_->getIStepInterval().step;
        int start = inlfld_->getIStepInterval().start;
        int stop = inlfld_->getIStepInterval().stop;
        hs.start_.inl() = step * (int)( Math::Ceil( (float)start/(float)step ) );
        if (hs.start_.inl()>start)
            hs.start_.inl() -= step;
        hs.stop_.inl()  = step * (int)( Math::Floor( (float)stop/(float)step ) );
        if (hs.stop_.inl()<stop)
            hs.stop_.inl() += step;
        hs.step_.inl()  = step;
        if (inlfld_->getIStepInterval() != hs.inlRange())
            inlfld_->setValue(hs.inlRange());
        
        step = crlfld_->getIStepInterval().step;
        start = crlfld_->getIStepInterval().start;
        stop = crlfld_->getIStepInterval().stop;
        hs.start_.crl() = step * (int)( Math::Ceil( (float)start/(float)step ) );
        if (hs.start_.crl()>start)
            hs.start_.crl() -= step;
        hs.stop_.crl()  = step * (int)( Math::Floor( (float)stop/(float)step ) );
        if (hs.stop_.crl()<stop)
            hs.stop_.crl() += step;
        hs.step_.crl()  = step;
        if (crlfld_->getIStepInterval() != hs.crlRange())
            crlfld_->setValue(hs.crlRange());
    }
}
