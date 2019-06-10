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

#include "seisioobjinfo.h"

#include "uiseis2dlineselgrp.h"
#include "mistieestimator.h"

uiMistieEstimateMainWin::uiMistieEstimateMainWin( uiParent* p )
    : uiDialog(p,uiDialog::Setup(getCaptionStr(),mNoDlgTitle,HelpKey("wgm","mistie")).modal(true) )
{
    setCtrlStyle( OkAndCancel );
    setOkText( uiStrings::sApply() );
    
    uiSeisSel::Setup su(Seis::Line);
    su.withinserters(false);
    seisselfld_ = new uiSeisSel( this,uiSeisSel::ioContext(Seis::Line,true), su );
    seisselfld_->selectionDone.notify(mCB(this,uiMistieEstimateMainWin,seisselCB));
    
    lineselfld_ = new WMLib::uiSeis2DLineSelGrp( this, OD::ChooseZeroOrMore );
    lineselfld_->attach(hCentered);
    lineselfld_->attach(ensureBelow, seisselfld_);
    
    uiString lagLabel(tr("Maximum Timeshift "));
    lagLabel.append(SI().zDomain().uiUnitStr(true));
    lagfld_ = new uiLabeledSpinBox( this, lagLabel );
    lagfld_->box()->setInterval(0, 250, 1);
    lagfld_->box()->setValue(100.0);
    lagfld_->attach( ensureBelow, lineselfld_ );
    
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
    
    
/*    BufferString defseldir = FilePath(GetDataDir()).add("Misc").fullPath();
    filefld_ = new uiFileInput( this, "Output file",
                                uiFileInput::Setup(uiFileDialog::Gen)
                                .forread(false).filter("*.mta").defseldir(defseldir).allowallextensions(false) );
    filefld_->setDefaultExtension( "mta" );
    filefld_->attach( hCentered );
    filefld_->attach( ensureBelow, lagfld_ );
*/    
    postFinalise().notify(mCB(this,uiMistieEstimateMainWin,seisselCB));
}

uiMistieEstimateMainWin::~uiMistieEstimateMainWin()
{}

bool uiMistieEstimateMainWin::acceptOK(CallBacker*)
{
/*    if (!strlen(filefld_->fileName())) {
        uiMSG().error( tr("Please specify an output file") );
        return false;
    }
*/    
    if (lineselfld_->nrChosen()==0) {
        uiMSG().error( tr("Please select the 2D line(s) to analyse") );
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
    
/*    if (!misties.getMisties().write(filefld_->fileName())) {
        ErrMsg("uiMistieEstimateMainWin::acceptOK - error writing misties to file");
        return false;
    }
*/    
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
