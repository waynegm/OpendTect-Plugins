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

#include "uimistieestimatemainwin.h"

static int sMnuID = 0;

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

uiMistieAnalysisMainWin::uiMistieAnalysisMainWin( uiParent* p )
: uiDialog(p, uiDialog::Setup(getCaptionStr(),mNoDlgTitle,HelpKey("wgm","mistie")).modal(false))
    , newitem_(uiStrings::sNew(), "new", "", mCB(this,uiMistieAnalysisMainWin,newCB), sMnuID++) 
    , openitem_(uiStrings::sOpen(), "open", "", mCB(this,uiMistieAnalysisMainWin,openCB), sMnuID++) 
    , saveitem_(uiStrings::sSave(), "save", "", mCB(this,uiMistieAnalysisMainWin,saveCB), sMnuID++) 
    , saveasitem_(uiStrings::sSaveAs(), "saveas", "", mCB(this,uiMistieAnalysisMainWin,saveasCB), sMnuID++)
    , mergeitem_(uiStrings::sMerge(), "plus", "", mCB(this,uiMistieAnalysisMainWin,mergeCB), sMnuID++)
    , calcitem_(tr("Calculate Corrections"), "launch", "", mCB(this,uiMistieAnalysisMainWin,calcCB), sMnuID++)
    , helpitem_(tr("Help"), "contexthelp", "", mCB(this,uiMistieAnalysisMainWin,helpCB), sMnuID++) 
    
{
    setCtrlStyle(CloseOnly);
    createToolBar();
    
    table_ = new uiTable( this, uiTable::Setup().rowgrow(true).selmode(uiTable::Multi),"Mistie Table" );
    BufferStringSet lbls(ColumnLabels);
    lbls.get(zCol) += SI().getZUnitString();
    table_->setColumnLabels( lbls);
    table_->showGrid( true );
    table_->setNrRows( 10 );
    table_->setLeftHeaderHidden( true );
    table_->setPrefWidthInChars(80);
    table_->setPrefHeightInRows(10);
    table_->setTableReadOnly(true);
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
    tb_->addSeparator();
    tb_->addButton( calcitem_ );
    tb_->addSeparator();
    tb_->addButton( helpitem_ );
}

void uiMistieAnalysisMainWin::helpCB( CallBacker* cb )
{
    HelpProvider::provideHelp( HelpKey("wgm","mistie") );
}

void uiMistieAnalysisMainWin::calcCB( CallBacker* cb )
{
    uiMSG().message("Calculate");
}

void uiMistieAnalysisMainWin::mergeCB( CallBacker* )
{
/*    uiMergeCorDlg dlg(this);
    if (!dlg.go())
        return;
    
    MistieCorrectionData merge;
    if (!merge.read(dlg.fileName())) {
        ErrMsg("uiMistieCorrMainWin::mergeCB - error reading misite correction file");
        return;
    }
    
    BufferStringSet existing;
    TypeSet<int> row;
    for (int idx=0; idx<table_->nrRows(); idx++) {
        BufferString lnm(table_->text(RowCol(idx, lineCol)));
        if (!lnm.isEmpty()) {
            existing.add(lnm);
            row += idx;
        }
    }
    for (int idx=0; idx<merge.size(); idx++) {
        int irow = existing.indexOf(merge.getDataName(idx));
        if (irow>=0 && dlg.replace()) {
            table_->setValue(RowCol(row[irow], shiftCol), merge.getZCor(idx));
            table_->setValue(RowCol(row[irow], phaseCol), merge.getPhaseCor(idx));
            table_->setValue(RowCol(row[irow], ampCol), merge.getAmpCor(idx));
        } else {
            int newRow = table_->nrRows();
            table_->insertRows(newRow, 1);
            table_->setText(RowCol(newRow, lineCol), merge.getDataName(idx));
            table_->setValue(RowCol(newRow, shiftCol), merge.getZCor(idx));
            table_->setValue(RowCol(newRow, phaseCol), merge.getPhaseCor(idx));
            table_->setValue(RowCol(newRow, ampCol), merge.getAmpCor(idx));
        }
    }
    raise();*/
}

void uiMistieAnalysisMainWin::newCB( CallBacker* )
{
    filename_.setEmpty();
    
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
    
    for (int idx=0; idx<misties_.size(); idx++) {
        misties_.get(idx, lineA, trcA, lineB, trcB, pos, zdiff, phasediff, ampdiff, quality);
        table_->setText(RowCol(idx, lineACol), lineA);
        table_->setText(RowCol(idx, lineBCol), lineB);
        table_->setValue(RowCol(idx, trcACol), trcA);
        table_->setValue(RowCol(idx, trcBCol), trcB);
        table_->setValue(RowCol(idx, xCol), pos.x);
        table_->setValue(RowCol(idx, yCol), pos.y);
        table_->setValue(RowCol(idx, zCol), zdiff);
        table_->setValue(RowCol(idx, phaseCol), phasediff);
        table_->setValue(RowCol(idx, ampCol), ampdiff);
        table_->setValue(RowCol(idx, qualCol), quality);
    }
    table_->setPrefHeightInRows(50);
}

void uiMistieAnalysisMainWin::openCB( CallBacker* )
{
    BufferString defseldir = FilePath(GetDataDir()).add(MistieData::defDirStr()).fullPath();
    uiFileDialog dlg( this, true, 0, MistieData::filtStr(), tr("Load Mistie File") );
    dlg.setDirectory(defseldir);
    if (!dlg.go())
        return;

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
    
    if (!misties_.write( filename_))
        ErrMsg("uiMistieAnalysisMainWin::saveCB - error saving misties to file");
}

void uiMistieAnalysisMainWin::saveasCB( CallBacker* )
{
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

uiString uiMistieAnalysisMainWin::getCaptionStr() const
{
    return tr("Mistie Analysis");
}
