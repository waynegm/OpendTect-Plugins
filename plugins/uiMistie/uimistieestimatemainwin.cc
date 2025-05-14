#include "uimistieestimatemainwin.h"

#include "ctxtioobj.h"
#include "emsurfacetr.h"
#include "emhorizon3d.h"
#include "emmanager.h"
#include "emobject.h"
#include "datainpspec.h"
#include "geom2dintersections.h"
#include "iodir.h"
#include "iodirentry.h"
#include "ioman.h"
#include "ioman.h"
#include "seis2ddata.h"
#include "string.h"
#include "seisselectionimpl.h"
#include "survgeom.h"
#include "survinfo.h"
#include "zdomain.h"
#include "uibutton.h"
#include "uidialog.h"
#include "uigeninput.h"
#include "uimsg.h"
#include "uiseissel.h"
#include "uispinbox.h"
#include "uistring.h"
#include "uistrings.h"
#include "uitaskrunner.h"

#include "uiseis2dlineselgrp.h"
#include "uihorinputgrp.h"
#include "mistieestimator.h"
#include "mistieestimator2d3d.h"


uiMistieEstimateBySeismic::uiMistieEstimateBySeismic( uiParent* p )
: uiGroup(p)
{
    uiObject* lastfld = nullptr;

    if (SI().has2D()) {
	uiSeisSel::Setup su(Seis::Line);
	su.withinserters(false);
	su.seltxt(tr("Input 2D Data Type/Attribute"));
	seisselfld_ = new uiSeisSel( this,uiSeisSel::ioContext(Seis::Line,true), su );
	mAttachCB(seisselfld_->selectionDone, uiMistieEstimateBySeismic::seisselCB);

	lineselfld_ = new WMLib::uiSeis2DLineSelGrp( this, OD::ChooseZeroOrMore );
	lineselfld_->attach(hCentered);
	lineselfld_->attach(ensureBelow, seisselfld_);
	lastfld = (uiObject*) lineselfld_->attachObj();
    }

    if (SI().has3D()) {
	use3dfld_ = new uiCheckBox(this, tr("Include 3D data"));
	if (lastfld)
	    use3dfld_->attach(alignedBelow, lastfld);
	use3dfld_->setChecked(false);
	mAttachCB(use3dfld_->activated, uiMistieEstimateBySeismic::use3DCB);

	uiSeisSel::Setup sud(Seis::Vol);
	sud.seltxt(tr("Input 3D Data Type/Attribute"));
	data3dfld_ = new uiSeisSel(this, uiSeisSel::ioContext(Seis::Vol,true), sud);
	data3dfld_->attach(ensureBelow, use3dfld_);

	trcstepfld_ = new uiGenInput(this, tr("2D Trace Step"), IntInpSpec(100));
	trcstepfld_->attach(alignedBelow, data3dfld_);

	lastfld = (uiObject*) trcstepfld_;
	use3DCB(nullptr);
    }

    uiString lagLabel(tr("Maximum Shift "));
    lagLabel.append(SI().zDomain().uiUnitStr(true));
    lagfld_ = new uiLabeledSpinBox( this, lagLabel );
    if ( SI().zIsTime() )
    {
	lagfld_->box()->setInterval(0, 250, 1);
	lagfld_->box()->setValue(100.0);
    }
    else if ( SI().zInMeter() )
    {
	lagfld_->box()->setInterval(0, 400, 1);
	lagfld_->box()->setValue(150.0);
    }
    else
    {
	lagfld_->box()->setInterval(0,1300, 1);
	lagfld_->box()->setValue(300.0);
    }


    if (lastfld)
	lagfld_->attach( ensureBelow, lastfld );

    uiString gateLabel(tr("Mistie "));
    gateLabel.append(uiStrings::sAnalysis());
    gateLabel.append(SI().zDomain().getRange());
    gateLabel.append(SI().zDomain().uiUnitStr(true));
    gatefld_ = new uiGenInput( this, gateLabel, FloatInpIntervalSpec().setName("Z start",0).setName("Z stop",1) );
    gatefld_->attach( rightOf, lagfld_ );
    ZGate zr;
    if ( SI().zIsTime() )
	zr.set( 0.5, 1.5 );
    else if ( SI().zInMeter() )
	zr.set( 750, 2500 );
    else
	zr.set( 2400, 8200 );

    zr.limitTo(SI().zRange(false));
    zr.scale( (float)SI().zDomain().userFactor() );
    gatefld_->setValue( zr );
    mAttachCB(gatefld_->valueChanged, uiMistieEstimateBySeismic::gatefldchangeCB);

    onlyzfld_ = new uiCheckBox(this, tr("Only estimate Z misties"));
    onlyzfld_->attach(alignedBelow, lagfld_);
    onlyzfld_->setChecked(false);

    mAttachCB(postFinalize(), uiMistieEstimateBySeismic::seisselCB);
}

uiMistieEstimateBySeismic::~uiMistieEstimateBySeismic()
{
    detachAllNotifiers();
}

void uiMistieEstimateBySeismic::use3DCB(CallBacker*)
{
    if ( data3dfld_ && trcstepfld_) {
	data3dfld_->setSensitive(use3dfld_->isChecked());
	trcstepfld_->setSensitive(use3dfld_->isChecked());
    }
}

void uiMistieEstimateBySeismic::seisselCB(CallBacker*)
{
    if (!seisselfld_ || !lineselfld_)
	return;
    const IOObj* ioobj = seisselfld_->ioobj(true);
    if (ioobj) {
	Seis2DDataSet dset( *ioobj );
	BufferStringSet lnms;
	TypeSet<Pos::GeomID> geomids;
	dset.getGeomIDs(geomids);
	dset.getLineNames(lnms);
	lineselfld_->setInput(lnms, geomids);
    }
}

void uiMistieEstimateBySeismic::gatefldchangeCB(CallBacker*)
{
    ZGate zg = gatefld_->getFInterval();
    zg.sort();
    ZGate zr = SI().zRange(false);
    float zfac = SI().showZ2UserFactor();
    zg.limitTo(ZGate(zr.start_*zfac, zr.stop_*zfac));
    gatefld_->setValue(zg);
}

bool uiMistieEstimateBySeismic::estimateMisties(MistieData& misties)
{
    if (!lineselfld_ || !seisselfld_) {
	uiMSG().error( tr("Mistie analysis requires 2D data") );
	return false;
    }

    if (lineselfld_ && lineselfld_->nrChosen()==0) {
	uiMSG().error( tr("Please select the 2D line(s) to analyse") );
	return false;
    }

    if (trcstepfld_ && trcstepfld_->getIntValue()<=0) {
	uiMSG().error(tr("The 2D Trace Step must be a positive number."));
	return false;
    }

    ZGate zrg = gatefld_->getFInterval();
    zrg.scale(1.0/SI().showZ2UserFactor());

    float lagtime = lagfld_->box()->getFValue() / SI().showZ2UserFactor();

    TypeSet<Pos::GeomID> geomids;
    lineselfld_->getChosen(geomids);
    ManagedObjectSet<BendPoints> bendpoints;

    uiTaskRunner uitr(this);
    BendPointFinder2DGeomSet bpfinder(geomids, bendpoints);
    TaskRunner::execute(&uitr, bpfinder);

    Line2DInterSectionSet intset;
    Line2DInterSectionFinder intfinder(bendpoints,  intset);
    TaskRunner::execute(&uitr, intfinder);

    MistieEstimatorFromSeismic misest(seisselfld_->ioobj(true), intset, zrg, lagtime, !onlyzfld_->isChecked());
    TaskRunner::execute(&uitr, misest);
    misties = misest.getMisties();

    if (trcstepfld_ && data3dfld_ && use3dfld_ && use3dfld_->isChecked()) {
	Line3DOverlapFinder lines3Doverlap(data3dfld_->ioobj(true), bendpoints);
	TaskRunner::execute(&uitr, lines3Doverlap);
	MistieEstimatorFromSeismic2D3D misties2d3d(data3dfld_->ioobj(true), seisselfld_->ioobj(true), lines3Doverlap.selData(), zrg, lagtime,
					trcstepfld_->getIntValue(), !onlyzfld_->isChecked());
	TaskRunner::execute(&uitr, misties2d3d);
	misties.add(misties2d3d.getMisties());
    }
    return true;
}


uiMistieEstimateByHorizon::uiMistieEstimateByHorizon(uiParent* p )
: uiGroup(p)
, trcstepfld_(nullptr)
{
    const bool has2Dhorizon = WMLib::uiHorInputGrp::has2Dhorizons();
    const bool has3Dhorizon = WMLib::uiHorInputGrp::has3Dhorizons();
    horinpgrp_ = new WMLib::uiHorInputGrp(this, has2Dhorizon, has3Dhorizon, false);
    if (has2Dhorizon && has3Dhorizon)
    {
	mAttachCB(horinpgrp_->exp3D_->activated, uiMistieEstimateByHorizon::use3DCB);
        trcstepfld_ = new uiGenInput(this, tr("2D Trace Step"), IntInpSpec(100));
	trcstepfld_->attach(rightOf, horinpgrp_->exp3D_);
	trcstepfld_->attach(ensureBelow, horinpgrp_);
	trcstepfld_->setSensitive(horinpgrp_->exp3D_->isChecked());
    }
}

uiMistieEstimateByHorizon::~uiMistieEstimateByHorizon()
{
    detachAllNotifiers();
}

void uiMistieEstimateByHorizon::use3DCB(CallBacker*)
{
    if (trcstepfld_)
	trcstepfld_->setSensitive(horinpgrp_->exp3D_->isChecked());
}


bool uiMistieEstimateByHorizon::estimateMisties(MistieData& misties)
{
    if (horinpgrp_->num2DLinesChosen()==0) {
	uiMSG().error( tr("Please select the 2D line(s) to analyse") );
	return false;
    }
    if (trcstepfld_ && trcstepfld_->getIntValue()<=0) {
	uiMSG().error(tr("The 2D Trace Step must be a positive number."));
	return false;
    }
    MultiID hor2Did, hor3Did;
    horinpgrp_->getHorIds(hor2Did, hor3Did);
    if (hor2Did.isUdf()) {
	uiMSG().error( tr("Please select a 2D horizon to analyse") );
	return false;
    }
    if (hor3Did.isUdf() && horinpgrp_->exp3D_ && horinpgrp_->exp3D_->isChecked()) {
	uiMSG().error( tr("Please select a 3D horizon to analyse") );
	return false;
    }

    TypeSet<Pos::GeomID> geomids;
    horinpgrp_->getGeoMids(geomids);
    ManagedObjectSet<BendPoints> bendpoints;

    uiTaskRunner uitr(this);
    BendPointFinder2DGeomSet bpfinder(geomids, bendpoints);
    TaskRunner::execute(&uitr, bpfinder);

    Line2DInterSectionSet intset;
    Line2DInterSectionFinder intfinder(bendpoints,  intset);
    TaskRunner::execute(&uitr, intfinder);

    MistieEstimatorFromHorizon misest(hor2Did, intset);
    TaskRunner::execute(&uitr, misest);
    misties = misest.getMisties();

    if (!hor3Did.isUdf() && horinpgrp_->exp3D_ && horinpgrp_->exp3D_->isChecked()) {
	EM::EMObject* obj3d = EM::EMM().loadIfNotFullyLoaded(hor3Did);
	if (!obj3d) {
	    ErrMsg("uiMistieEstimateByHorizon::estimateMisties - loading 3D input horizon failed");
	    return false;
	}
	obj3d->ref();
	EM::Horizon3D* hor3d;
	mDynamicCast(EM::Horizon3D*,hor3d,obj3d);
	if (!hor3d)
	{
	    ErrMsg("uiMistieEstimateByHorizon::estimateMisties - casting horizons failed");
	    obj3d->unRef();
	    return false;
	}
	Line3DOverlapFinder lines3Doverlap(hor3d, bendpoints);
	TaskRunner::execute(&uitr, lines3Doverlap);
	obj3d->unRef();
	if (trcstepfld_) {
	    MistieEstimatorFromHorizon2D3D misties2d3d(hor3Did, hor2Did, lines3Doverlap.selData(), trcstepfld_->getIntValue());
	    TaskRunner::execute(&uitr, misties2d3d);
	    misties.add(misties2d3d.getMisties());
	}
    }

    return true;
}

uiMistieEstimateMainWin::uiMistieEstimateMainWin(uiParent* p)
    : uiDialog(p,uiDialog::Setup(getCaptionStr(),mNoDlgTitle,HelpKey("wgm","mistie")).modal(true) )
{
    setCtrlStyle(OkAndCancel);
    setOkText(uiStrings::sApply());

    BoolInpSpec bis(true, tr("Seismic Traces"), tr("Horizon Interpretation"));
    fromseisfld_ = new uiGenInput(this, tr("Estimate misties from"), bis);
    mAttachCB(fromseisfld_->valueChanged, uiMistieEstimateMainWin::modeCB);

    seisgrp_ = new uiMistieEstimateBySeismic(this);
    seisgrp_->attach(alignedBelow, fromseisfld_);

    horgrp_ = new uiMistieEstimateByHorizon( this );
    horgrp_->attach(alignedBelow, fromseisfld_);

    mAttachCB(postFinalize(), uiMistieEstimateMainWin::initCB);
}

uiMistieEstimateMainWin::~uiMistieEstimateMainWin()
{
    detachAllNotifiers();
}

bool uiMistieEstimateMainWin::acceptOK(CallBacker*)
{
    const bool fromseis = fromseisfld_->getBoolValue();
    bool result = false;
    if (fromseis)
	result = seisgrp_->estimateMisties(misties_);
    else
	result = horgrp_->estimateMisties(misties_);

    return result;
}

uiString uiMistieEstimateMainWin::getCaptionStr() const
{
    return tr("Mistie Estimation");
}

void uiMistieEstimateMainWin::modeCB(CallBacker*)
{
    const bool fromseis = fromseisfld_->getBoolValue();
    seisgrp_->display(fromseis);
    horgrp_->display(!fromseis);
}

void uiMistieEstimateMainWin::initCB(CallBacker*)
{
    modeCB(nullptr);
}


