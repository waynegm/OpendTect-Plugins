#include "uimistieanalysismainwin.h"

#include "uimain.h"
#include "uimenu.h"
#include "uimsg.h"
#include "uistrings.h"
#include "uitoolbar.h"
#include "helpview.h"
#include "uitable.h"
#include "survinfo.h"
#include "seisioobjinfo.h"
#include "bufstringset.h"
#include "uifiledlg.h"
#include "filepath.h"
#include "oddirs.h"
#include "uibuttongroup.h"
#include "uibutton.h"
#include "uifileinput.h"
#include "uilabel.h"
#include "uidialog.h"
#include "string.h"
#include "uistrings.h"
#include "uispinbox.h"
#include "od_ostream.h"
#include "uidesktopservices.h"

#include "uimistieestimatemainwin.h"
#include "mistiecordata.h"
#include "uibufferstringsetselgrp.h"
#include "uicorrviewer.h"

static int sMnuID = 0;

#include "mistie_report.h" 


static const char* ColumnLabels[] =
{
    "LineA",
    "LineB",
    "TrcA",
    "TrcB",
    "X",
    "Y",
    "Z Mistie\n",
    "Phase\nRotation (deg)",
    "Amplitude\nScalar",
    "Quality",
    0
};

class uiMergeEstDlg : public uiDialog
{ mODTextTranslationClass(uiMergeEstDlg);
public:
    uiMergeEstDlg( uiParent *p )
    : uiDialog(p,Setup(tr("Merge Other Mistie File"),mNoDlgTitle,mTODOHelpKey))
    {
        BufferString defseldir = FilePath(GetDataDir()).add(MistieData::defDirStr()).fullPath();
        filefld_ = new uiFileInput( this, tr("Mistie File"), uiFileInput::Setup(uiFileDialog::Gen)
        .forread(true)
        .defseldir(defseldir)
        .filter(MistieData::filtStr()) );
        filefld_->setElemSzPol(uiObject::WideVar);
        
        actiongrp_ = new uiButtonGroup( this, "", OD::Horizontal );
        actiongrp_->setExclusive( true );
        actiongrp_->attach( alignedBelow, filefld_ );
        keepbut_ = new uiCheckBox( actiongrp_, tr("Keep") );
        keepbut_->setChecked(true);
        replacebut_ = new uiCheckBox( actiongrp_, tr("Replace existing") );
        uiLabel* lbl = new uiLabel( this, tr("Merge and ") );
        lbl->attach( centeredLeftOf, actiongrp_ );
        
        setOkText(uiStrings::sMerge());
    }
    
    BufferString    fileName() const { return filefld_->fileName(); }
    bool            replace() const { return replacebut_->isChecked(); }
    
protected:
    bool acceptOK( CallBacker* )
    {
        if (!strlen(filefld_->fileName())) {
            uiMSG().error( tr("Please specify an input file to merge") );
            return false;
        }
        return true;
    }
    
    uiFileInput*    filefld_;
    uiButtonGroup*  actiongrp_;
    uiCheckBox*     keepbut_;
    uiCheckBox*     replacebut_;
};

class uiFilterEstDlg : public uiDialog
{ mODTextTranslationClass(uiFilterEstDlg);
public:
    uiFilterEstDlg( uiParent *p, float minquality )
    : uiDialog(p,Setup(tr("Filters on Mistie Estimates"),mNoDlgTitle,mTODOHelpKey))
    {
	qualfld_ = new uiGenInput( this, tr("Minimum Tie Quality"), FloatInpSpec(0.5, 0.0, 1.0, 0.1) );
	qualfld_->setWithCheck();
	if (!mIsUdf(minquality)) {
	    qualfld_->setValue(minquality);
	    qualfld_->setChecked(true);
	}

	setOkText(uiStrings::sFilter());
    }

    uiGenInput* 	qualfld_;
    protected:
    bool acceptOK( CallBacker* )
    {
	return true;
    }
};

class uiCorrCalcDlg : public uiDialog
{ mODTextTranslationClass(uiCorrCalcDlg);
public:
    uiCorrCalcDlg( uiParent* p, const MistieData& misties, const float minquality, MistieCorrectionData& corrs )
    : uiDialog(p,Setup(tr("Calculate Mistie Corrections"),mNoDlgTitle,mTODOHelpKey))
    , misties_(misties)
    , corrs_(corrs)
    , minquality_(minquality)
    {
        
        lineselfld_ = new WMLib::uiBufferStringSetSelGrp( this, OD::ChooseZeroOrMore );
        BufferStringSet lines;
        misties_.getAllLines(lines);
        lineselfld_->setInput(lines);

        uiLabel* lbl = new uiLabel( this, tr("Reference Line(s)") );
        lbl->attach(leftOf, lineselfld_);
        
        maxiterfld_ = new uiLabeledSpinBox(this, tr("Maximum Iterations"));
        maxiterfld_->box()->setInterval(1, 500, 1);
        maxiterfld_->box()->setValue( 200 );
        maxiterfld_->attach(alignedBelow, lineselfld_);
        
        uiString minzchglbl(tr("Minimum RMS Z Mistie Change "));
        minzchglbl.append(SI().zDomain().uiUnitStr(true));
        minzchgfld_ = new uiGenInput(this, minzchglbl, FloatInpSpec(0.01));
        minzchgfld_->attach(alignedBelow, maxiterfld_);
        minzchgfld_->valuechanged.notify(mCB(this,uiCorrCalcDlg,minchgCB));

        minphasechgfld_ = new uiGenInput(this, tr("Minimum RMS Phase Change (deg)"), FloatInpSpec(0.01));
        minphasechgfld_->attach(alignedBelow, minzchgfld_);
        minphasechgfld_->valuechanged.notify(mCB(this,uiCorrCalcDlg,minchgCB));
        
        minampchgfld_ = new uiGenInput(this, tr("Minimum RMS Amplitude Change"), FloatInpSpec(0.001));
        minampchgfld_->attach(alignedBelow, minphasechgfld_);
        minampchgfld_->valuechanged.notify(mCB(this,uiCorrCalcDlg,minchgCB));

        setOkText(uiStrings::sCalculate());
    }
    
protected:
    const MistieData&           misties_;
    MistieCorrectionData&       corrs_;
    float			minquality_;
    
    WMLib::uiBufferStringSetSelGrp*  lineselfld_;
    uiLabeledSpinBox*           maxiterfld_;
    uiGenInput*                 minzchgfld_;
    uiGenInput*                 minampchgfld_;
    uiGenInput*                 minphasechgfld_;
    
    void minchgCB(CallBacker*)
    {
        if (minzchgfld_->getfValue()<0.0)
            minzchgfld_->setValue(0.0);
        if (minphasechgfld_->getfValue()<0.0)
            minphasechgfld_->setValue(0.0);
        if (minampchgfld_->getfValue()<0.0)
            minampchgfld_->setValue(0.0);
    }
    
    bool acceptOK( CallBacker* )
    {
        BufferStringSet lnms;
        lineselfld_->getChosen(lnms);
        corrs_.erase();
        corrs_.computeZCor(misties_, lnms, minquality_, maxiterfld_->box()->getValue(), 0.75, minzchgfld_->getfValue());
        corrs_.computePhaseCor(misties_, lnms, minquality_, maxiterfld_->box()->getValue(), 0.75, minphasechgfld_->getfValue());
        corrs_.computeAmpCor(misties_, lnms, minquality_, maxiterfld_->box()->getValue(), 0.75, minampchgfld_->getfValue());
        
        return true;
    }
};

uiMistieAnalysisMainWin::uiMistieAnalysisMainWin( uiParent* p )
    : uiDialog(p, uiDialog::Setup(getCaptionStr(),mNoDlgTitle,HelpKey("wgm","mistie")).modal(false))
    , newitem_(uiStrings::sNew(), "new", "", mCB(this,uiMistieAnalysisMainWin,newCB), sMnuID++) 
    , openitem_(uiStrings::sOpen(), "open", "", mCB(this,uiMistieAnalysisMainWin,openCB), sMnuID++) 
    , saveitem_(uiStrings::sSave(), "save", "", mCB(this,uiMistieAnalysisMainWin,saveCB), sMnuID++) 
    , saveasitem_(uiStrings::sSaveAs(), "saveas", "", mCB(this,uiMistieAnalysisMainWin,saveasCB), sMnuID++)
    , mergeitem_(uiStrings::sMerge(), "plus", "", mCB(this,uiMistieAnalysisMainWin,mergeCB), sMnuID++)
    , filteritem_(uiStrings::sFilter(), "minus", "", mCB(this,uiMistieAnalysisMainWin,filterCB), sMnuID++)
    , calcitem_(tr("Calculate Corrections"), "attributes", "", mCB(this,uiMistieAnalysisMainWin,calcCB), sMnuID++)
    , xplotitem_(tr("Crossplot Mistie Data"), "xplot", "", mCB(this,uiMistieAnalysisMainWin,xplotCB), sMnuID++)
    , helpitem_(tr("Help"), "contexthelp", "", mCB(this,uiMistieAnalysisMainWin,helpCB), sMnuID++) 
    , corrviewer_(0)
    
{
    setCtrlStyle(CloseOnly);
    createToolBar();
    mSetUdf(minquality_);
    
    table_ = new uiTable( this, uiTable::Setup().rowgrow(false).selmode(uiTable::Multi),"Mistie Table" );
    BufferStringSet lbls(ColumnLabels);
    lbls.get(zCol) += SI().getZUnitString();
    table_->setColumnLabels( lbls);
    table_->showGrid( true );
    table_->setNrRows( 10 );
    table_->setLeftHeaderHidden( true );
    table_->setPrefWidthInChars(80);
    table_->setPrefHeightInRows(10);
    table_->setTableReadOnly(true);
    
    windowClosed.notify(mCB(this,uiMistieAnalysisMainWin,closeCB));
}

uiMistieAnalysisMainWin::~uiMistieAnalysisMainWin()
{
}

void uiMistieAnalysisMainWin::createToolBar()
{
    tb_ = new uiToolBar( this, tr("Mistie Analysis Viewer") );
    tb_->addButton( newitem_ );
    tb_->addButton( openitem_ );
    tb_->addButton( saveitem_ );
    tb_->addButton( saveasitem_ );
    tb_->addSeparator();
    tb_->addButton( mergeitem_ );
    tb_->addButton( filteritem_ );
    tb_->addSeparator();
    tb_->addButton( calcitem_ );
    tb_->addButton( xplotitem_ );
    tb_->addSeparator();
    tb_->addButton( helpitem_ );
}

bool uiMistieAnalysisMainWin::checkMistieSave()
{
    if (misties_.size()>0 && filename_.isEmpty()) {
        uiString msg = tr("The latest misties have not been saved.\nDo you want to save them?");
        int result = uiMSG().askSave(msg, false);
        if (result == 1)
            saveCB(0);
    }
    return true;
}

void uiMistieAnalysisMainWin::closeCB(CallBacker*)
{
    if (!checkMistieSave())
        return;

    if (corrviewer_) {
        corrviewer_->close();
	deleteAndZeroPtr( corrviewer_ );
    }
    
    misties_.erase();
    corrs_.erase();
    filename_.setEmpty();
}

void uiMistieAnalysisMainWin::helpCB( CallBacker* cb )
{
    HelpProvider::provideHelp( HelpKey("wgm","mistie") );
}

void uiMistieAnalysisMainWin::calcCB( CallBacker* cb )
{
    uiCorrCalcDlg dlg(this, misties_, mIsUdf(minquality_) ? 0.0 : minquality_, corrs_);
    if (!dlg.go())
        return;
    
    if (!corrviewer_)
        corrviewer_ = new uiCorrViewer(this, corrs_);
    corrviewer_->newCorrData();
    corrviewer_->show();
    corrviewer_->raise();
}

void uiMistieAnalysisMainWin::xplotCB( CallBacker* )
{
    BufferString fnm = FilePath::getTempName("html");
    FilePath outputfp( fnm );
    {
        BufferString lineA, lineB;
        int trcA, trcB;
        float zdiff, phasediff, ampdiff, quality;
        Coord pos;

        BufferStringSet lbls(ColumnLabels);
        lbls.get(zCol) += SI().getZUnitString();
        
        od_ostream strm(outputfp.fullPath());
        if (!strm.isOK())
            return;
        strm << mistie_report;
        strm << "<script>\n";
        strm << "var labels = [ \"" << lbls.get(0).remove("\n") << "\"";
        for (int idx=1; idx<lbls.size(); idx++) 
            strm << " , \"" << lbls.get(idx).remove("\n") << "\"";
        if (corrs_.size() > 0)
            strm << " , \"Res Z\", \"Res Phase\", \"Res Amp\" ";
        strm << "];\n";
        strm << "var misties = [ \n";
        for (int idx=0; idx<misties_.size(); idx++) {
            misties_.get(idx, lineA, trcA, lineB, trcB, pos, zdiff, phasediff, ampdiff, quality);
	    if ( !mIsUdf(minquality_) && quality < minquality_ )
		continue;
            strm << "[ \"" << lineA << "\" , " << trcA << ", \"" << lineB <<"\", " << trcB << ", " << pos.x << ", " << pos.y << ", " << zdiff << ", " << phasediff << ", " << ampdiff << ", " << quality;
            if (corrs_.size()>0)
                strm << ", " << misties_.getZMistieWith(corrs_, idx) << ", " << misties_.getPhaseMistieWith(corrs_,idx) << ", " << misties_.getAmpMistieWith(corrs_, idx);
	    strm << "],\n";
        }
        strm << "];\n</script>\n</body>\n</html>\n";
    }
    uiDesktopServices::openUrl ( outputfp.fullPath() );
}

void uiMistieAnalysisMainWin::mergeCB( CallBacker* )
{
    uiMergeEstDlg dlg(this);
    if (!dlg.go())
        return;
    
    corrs_.erase();
    if (!misties_.read(dlg.fileName(), true, dlg.replace())) {
        ErrMsg("uiMistieAnalysisMainWin::mergeCB - error reading mistie file");
        return;
    }
    
    fillTable();
}

void uiMistieAnalysisMainWin::filterCB( CallBacker* )
{
    uiFilterEstDlg dlg(this, minquality_);
    if (!dlg.go())
	return;

    if ( dlg.qualfld_->isChecked() )
	minquality_ = dlg.qualfld_->getFValue();
    else
	mSetUdf(minquality_);

    fillTable();
}

void uiMistieAnalysisMainWin::newCB( CallBacker* )
{
    if (!checkMistieSave())
        return;

    if (corrviewer_)
        corrviewer_->close();
    
    filename_.setEmpty();
    corrs_.erase();
    
    uiMistieEstimateMainWin* dlg = new uiMistieEstimateMainWin(this);
    if (!dlg->go())
        return;
    
    misties_ = dlg->getMisties();
    fillTable();
}

void uiMistieAnalysisMainWin::fillTable()
{
    table_->clearTable();
    table_->setNrRows(misties_.size());
    BufferString lineA, lineB;
    int trcA, trcB;
    float zdiff, phasediff, ampdiff, quality;
    Coord pos;
    
    int irow = 0;
    for (int idx=0; idx<misties_.size(); idx++) {
        misties_.get(idx, lineA, trcA, lineB, trcB, pos, zdiff, phasediff, ampdiff, quality);
	if ( !mIsUdf(minquality_) && quality<minquality_ )
	    continue;
        table_->setText(RowCol(irow, lineACol), lineA);
        table_->setText(RowCol(irow, lineBCol), lineB);
        table_->setValue(RowCol(irow, trcACol), trcA);
        table_->setValue(RowCol(irow, trcBCol), trcB);
        table_->setValue(RowCol(irow, xCol), pos.x, 1);
        table_->setValue(RowCol(irow, yCol), pos.y, 1);
        table_->setValue(RowCol(irow, zCol), zdiff, 2);
        table_->setValue(RowCol(irow, phaseCol), phasediff, 2);
        table_->setValue(RowCol(irow, ampCol), ampdiff, 2);
        table_->setValue(RowCol(irow, qualCol), quality, 3);
	irow++;
    }
    table_->setPrefHeightInRows(50);
}

void uiMistieAnalysisMainWin::openCB( CallBacker* )
{
    if (!checkMistieSave())
        return;

    if (corrviewer_)
        corrviewer_->close();
    
    BufferString defseldir = FilePath(GetDataDir()).add(MistieData::defDirStr()).fullPath();
    uiFileDialog dlg( this, true, 0, MistieData::filtStr(), tr("Load Mistie File") );
    dlg.setDirectory(defseldir);
    if (!dlg.go())
        return;
    
    corrs_.erase();
    filename_ = dlg.fileName();
    if (!misties_.read( filename_ )) {
        filename_.setEmpty();
        ErrMsg("uiMistieAnalysisMainWin::openCB - error reading mistie file");
        return;
    }
    
    fillTable();
}

void uiMistieAnalysisMainWin::saveCB( CallBacker* )
{
    if (filename_.isEmpty())
        saveasCB(0);
    
    if (misties_.size()>0)
        if (!misties_.write( filename_))
            ErrMsg("uiMistieAnalysisMainWin::saveCB - error saving misties to file");
}

void uiMistieAnalysisMainWin::saveasCB( CallBacker* )
{
    if (misties_.size()>0) {
        BufferString defseldir = FilePath(GetDataDir()).add(MistieData::defDirStr()).fullPath();
        uiFileDialog dlg( this, false, 0, MistieData::filtStr(), tr("Save Misties") );
        dlg.setMode(uiFileDialog::AnyFile);
        dlg.setDirectory(defseldir);
        dlg.setDefaultExtension(MistieData::extStr());
        dlg.setConfirmOverwrite(true);
        dlg.setSelectedFilter(MistieData::filtStr());
        if (!dlg.go())
            return;
    
        filename_ = dlg.fileName();
        raise();
        saveCB(0);
    }
}

uiString uiMistieAnalysisMainWin::getCaptionStr() const
{
    return tr("Mistie Analysis");
}
