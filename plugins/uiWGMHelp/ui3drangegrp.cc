#include "ui3drangegrp.h"
#include "survinfo.h"

#include "uigeninput.h"
#include "uidlggroup.h"
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
    mAttachCB(inlfld_->valuechanged, ui3DRangeGrp::rangeChg);

    iis.setName("Crl Start",0).setName("Crl Stop",1).setName("Crl step",2);
    crlfld_ = new uiGenInput( this, tr("Cross-line range"), iis );
    crlfld_->attach(alignedBelow, inlfld_);
    mAttachCB(crlfld_->valuechanged, ui3DRangeGrp::rangeChg);

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

void WMLib::ui3DRangeGrp::usePar( const IOPar& par )
{
    TrcKeySampling hs;
    hs.usePar(par);
    setTrcKeySampling(hs);
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

void WMLib::ui3DRangeGrp::rangeChg(CallBacker* cb)
{
    if (stepSnap_) {
	NotifyStopper ns1(inlfld_->valuechanged);
	NotifyStopper ns2(crlfld_->valuechanged);

        TrcKeySampling hs;
	hs.step_ = BinID(inlfld_->getIStepInterval().step,
			 crlfld_->getIStepInterval().step);
	SI().snapStep(hs.step_, BinID(-1,-1));
	BinID start(inlfld_->getIStepInterval().start,
		    crlfld_->getIStepInterval().start);
	SI().snap(start, BinID(-1,-1));
	BinID stop(inlfld_->getIStepInterval().stop,
		   crlfld_->getIStepInterval().stop);
	SI().snap(stop, BinID(1,1));
	StepInterval<int> inlrg(start.inl(), stop.inl(), hs.step_.first);
        inlrg.snap(inlrg.stop, OD::SnapUpward);
        StepInterval<int> crlrg(start.crl(), stop.crl(), hs.step_.second);
        crlrg.snap(crlrg.stop, OD::SnapUpward);
        inlfld_->setValue(inlrg);
	crlfld_->setValue(crlrg);
    }
}

void WMLib::ui3DRangeGrp::setSnap( bool yn )
{
    stepSnap_ = yn;
    rangeChg(0);
}
