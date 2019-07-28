#include "uimistieestimatemainwin.h"

#include "uimsg.h"
#include "filepath.h"
#include "oddirs.h"
#include "uifileinput.h"
#include "uidialog.h"
#include "string.h"
#include "uistring.h"
#include "uistrings.h"
#include "uitaskrunner.h"
#include "zdomain.h"
#include "survinfo.h"
#include "datainpspec.h"
#include "geom2dintersections.h"
#include "survgeom.h"
#include "uiseissel.h"
#include "seis2ddata.h"
#include "ioman.h"
#include "uispinbox.h"
#include "uibutton.h"

#include "seisselectionimpl.h"
#include "seisioobjinfo.h"

#include "uiseis2dlineselgrp.h"
#include "mistieestimator.h"
#include "mistieestimator2d3d.h"

uiMistieEstimateMainWin::uiMistieEstimateMainWin( uiParent* p )
    : uiDialog(p,uiDialog::Setup(getCaptionStr(),mNoDlgTitle,HelpKey("wgm","mistie")).modal(true) )
    , data3dfld_(0)
{
    setCtrlStyle( OkAndCancel );
    setOkText( uiStrings::sApply() );
    uiObject* lastfld = 0;
    
    uiSeisSel::Setup su(Seis::Line);
    su.withinserters(false);
    su.seltxt(tr("Input 2D Data Type/Attribute"));
    seisselfld_ = new uiSeisSel( this,uiSeisSel::ioContext(Seis::Line,true), su );
    seisselfld_->selectionDone.notify(mCB(this,uiMistieEstimateMainWin,seisselCB));
    
    lineselfld_ = new WMLib::uiSeis2DLineSelGrp( this, OD::ChooseZeroOrMore );
    lineselfld_->attach(hCentered);
    lineselfld_->attach(ensureBelow, seisselfld_);
    lastfld = (uiObject*) lineselfld_;
    
    if (SI().has3D()) {
        use3dfld_ = new uiCheckBox(this, "Include 3D data");
        use3dfld_->attach(alignedBelow, lineselfld_);
        use3dfld_->setChecked(false);
        use3dfld_->activated.notify(mCB(this, uiMistieEstimateMainWin, use3DCB));
        
        uiSeisSel::Setup sud(Seis::Vol);
        sud.seltxt(tr("Input 3D Data Type/Attribute"));
        data3dfld_ = new uiSeisSel(this, uiSeisSel::ioContext(Seis::Vol,true), sud);
        data3dfld_->attach(ensureBelow, use3dfld_);
        
        trcstepfld_ = new uiGenInput(this, tr("2D Trace Step"), IntInpSpec(100));
        trcstepfld_->attach(alignedBelow, data3dfld_);
                                     
        lastfld = (uiObject*) trcstepfld_;
        use3DCB(0);
    }
    
    uiString lagLabel(tr("Maximum Timeshift "));
    lagLabel.append(SI().zDomain().uiUnitStr(true));
    lagfld_ = new uiLabeledSpinBox( this, lagLabel );
    lagfld_->box()->setInterval(0, 250, 1);
    lagfld_->box()->setValue(100.0);
    lagfld_->attach( ensureBelow, lastfld );
    
    uiString gateLabel(tr("Mistie "));
    gateLabel.append(uiStrings::sAnalysis());
    gateLabel.append(SI().zDomain().getRange());
    gateLabel.append(SI().zDomain().uiUnitStr(true));
    gatefld_ = new uiGenInput( this, gateLabel, FloatInpIntervalSpec().setName("Z start",0).setName("Z stop",1) );
    gatefld_->attach( rightOf, lagfld_ );
    ZGate zr(0.5,1.5);
    zr.limitTo(SI().zRange(false));
    float zfac = SI().showZ2UserFactor();
    gatefld_->setValue(ZGate(zr.start*zfac, zr.stop*zfac));
    gatefld_->valuechanged.notify(mCB(this,uiMistieEstimateMainWin,gatefldchangeCB));
    
    postFinalise().notify(mCB(this,uiMistieEstimateMainWin,seisselCB));
}

uiMistieEstimateMainWin::~uiMistieEstimateMainWin()
{}

void uiMistieEstimateMainWin::use3DCB(CallBacker*)
{
    data3dfld_->setSensitive(use3dfld_->isChecked());
    trcstepfld_->setSensitive(use3dfld_->isChecked());
}

bool uiMistieEstimateMainWin::acceptOK(CallBacker*)
{
    if (lineselfld_->nrChosen()==0) {
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
    
    uiTaskRunner uitr(this);
    BendPointFinder2DGeomSet bpfinder(geomids);
    TaskRunner::execute(&uitr, bpfinder);
    
    Line2DInterSectionSet intset;
    Line2DInterSectionFinder intfinder(bpfinder.bendPoints(),  intset);
    TaskRunner::execute(&uitr, intfinder);
    
    MistieEstimator misties(seisselfld_->ioobj(true), intset, zrg, lagtime);
    TaskRunner::execute(&uitr, misties);
    misties_ = misties.getMisties();
    
    if (SI().has3D() && use3dfld_->isChecked()) {
        Line3DOverlapFinder lines3Doverlap(data3dfld_->ioobj(true), bpfinder.bendPoints());
        TaskRunner::execute(&uitr, lines3Doverlap);
        
        MistieEstimator2D3D misties2d3d(data3dfld_->ioobj(true), seisselfld_->ioobj(true), lines3Doverlap.selData(), zrg, lagtime, trcstepfld_->getIntValue());
        TaskRunner::execute(&uitr, misties2d3d);
        misties_.add(misties2d3d.getMisties());
    }
    
    return true;
}

uiString uiMistieEstimateMainWin::getCaptionStr() const
{
    return tr("Mistie Estimation");
}

void uiMistieEstimateMainWin::gatefldchangeCB(CallBacker*)
{
   ZGate zg = gatefld_->getFInterval();
   zg.sort();
   ZGate zr = SI().zRange(false);
   float zfac = SI().showZ2UserFactor();
   zg.limitTo(ZGate(zr.start*zfac, zr.stop*zfac));
   gatefld_->setValue(zg);
}

void uiMistieEstimateMainWin::seisselCB(CallBacker*)
{
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
