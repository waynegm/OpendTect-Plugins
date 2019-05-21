#include "uimistiecorrmainwin.h"

#include "uimain.h"
#include "uimenu.h"
#include "uimsg.h"
#include "uitoolbar.h"
#include "helpview.h"
#include "uitable.h"
#include "survinfo.h"
#include "seisioobjinfo.h"
#include "bufstringset.h"
#include "uifiledlg.h"
#include "filepath.h"
#include "oddirs.h"
#include "iopar.h"

static int sMnuID = 0;

static const char* ColumnLabels[] =
{
    "Line/Dataset",
    "Z Shift ",
    "Phase Rotation (deg)",
    "Amplitude Scalar",
    0
};

uiMistieCorrMainWin::uiMistieCorrMainWin( uiParent* p )
    : uiMainWin(p, getCaptionStr() )
    , newitem_(uiStrings::sNew(), "new", "", mCB(this,uiMistieCorrMainWin,newCB), sMnuID++) 
    , openitem_(uiStrings::sOpen(), "open", "", mCB(this,uiMistieCorrMainWin,openCB), sMnuID++) 
    , saveitem_(uiStrings::sSave(), "save", "", mCB(this,uiMistieCorrMainWin,saveCB), sMnuID++) 
    , saveasitem_(uiStrings::sSaveAs(), "saveas", "", mCB(this,uiMistieCorrMainWin,saveasCB), sMnuID++)
    , helpitem_(tr("Help"), "contexthelp", "", mCB(this,uiMistieCorrMainWin,helpCB), sMnuID++) 
    
{
    createToolBar();
    
    table_ = new uiTable( this, uiTable::Setup().rowgrow(true)
                                                .selmode(uiTable::Multi),
                          "Mistie Correction Table" );
    BufferStringSet lbls(ColumnLabels);
    lbls.get(1) += SI().getZUnitString();
    table_->setColumnLabels( lbls);
    table_->showGrid( true );
    table_->setLeftHeaderHidden( true );
    
    table_->rowInserted.notify( mCB(this,uiMistieCorrMainWin,newrowCB) );

    newCB(0);
}

uiMistieCorrMainWin::~uiMistieCorrMainWin()
{
    
}

void uiMistieCorrMainWin::createToolBar()
{
    tb_ = new uiToolBar( this, tr("Mistie Correction Editor") );
    tb_->addButton( newitem_ );
    tb_->addButton( openitem_ );
    tb_->addButton( saveitem_ );
    tb_->addButton( saveasitem_ );
    tb_->addSeparator();
    tb_->addButton( helpitem_ );
}

void uiMistieCorrMainWin::helpCB( CallBacker* cb )
{
    HelpProvider::provideHelp( HelpKey("wgm","mistie") );
}

void uiMistieCorrMainWin::newrowCB( CallBacker* )
{
    table_->setValue(RowCol(table_->currentRow(), 1), 10.5);
}

void uiMistieCorrMainWin::newCB( CallBacker* )
{
    table_->clearTable();
    filename_.setEmpty();
    if (SI().has2D()) {
        BufferStringSet lnms;
        TypeSet<Pos::GeomID> geomids;
        SeisIOObjInfo::getLinesWithData(lnms, geomids);
        table_->setNrRows(lnms.size());
        for (int idx=0; idx<lnms.size(); idx++) {
            table_->setText(RowCol(idx,0), lnms.get(idx));
            table_->setValue(RowCol(idx,1), 0.0);
            table_->setValue(RowCol(idx,2), 0.0);
            table_->setValue(RowCol(idx,3), 1.0);
        }
    }
}

void uiMistieCorrMainWin::openCB( CallBacker* )
{
    BufferString defseldir = FilePath(GetDataDir()).add("Misc").fullPath();
    uiFileDialog dlg( this, true, 0, "*.mst", tr("Load Mistie Corrections") );
    dlg.setDirectory(defseldir);
    if (!dlg.go())
        return;

    filename_ = dlg.fileName();
    IOPar misties;
    if (!misties.read( filename_, 0 )) {
        filename_.setEmpty();
        ErrMsg("uiMistieCorrMainWin::openCB - error reading mistie correction file");
        return;
    }
    
    table_->clearTable();
    table_->setNrRows(misties.size()-1);
    int irow = 0;
    for (int idx=0; idx<misties.size(); idx++) {
        BufferString lnm = misties.getKey(idx);
        float shift, phase, amp;
        if (!lnm.isEmpty()) {
            misties.get(lnm, shift, phase, amp);
            table_->setText(RowCol(irow,0), lnm);
            table_->setValue(RowCol(irow,1), shift);
            table_->setValue(RowCol(irow,2), phase);
            table_->setValue(RowCol(irow,3), amp);
            irow++;
        }
    }
    raise();
}

void uiMistieCorrMainWin::saveCB( CallBacker* )
{
    if (filename_.isEmpty())
        saveasCB(0);
    
    IOPar misties;
    for (int idx=0; idx<table_->nrRows(); idx++) {
        BufferString lnm(table_->text(RowCol(idx,0)));
        if (!lnm.isEmpty()) 
            misties.set( lnm, table_->getFValue(RowCol(idx,1)), table_->getFValue(RowCol(idx,2)),  table_->getFValue(RowCol(idx,3)) );
    }
    if (!misties.write( filename_, 0))
        ErrMsg("uiMistieCorrMainWin::saveCB - error saving mistie corrections to file");
}

void uiMistieCorrMainWin::saveasCB( CallBacker* )
{
    BufferString defseldir = FilePath(GetDataDir()).add("Misc").fullPath();
    uiFileDialog dlg( this, false, 0, "*.mst", tr("Save Mistie Corrections") );
    dlg.setMode(uiFileDialog::AnyFile);
    dlg.setDirectory(defseldir);
    dlg.setDefaultExtension("mst");
    dlg.setConfirmOverwrite(true);
    dlg.setSelectedFilter("*.mst");
    if (!dlg.go())
        return;
    
    filename_ = dlg.fileName();
    raise();
    saveCB(0);
}

uiString uiMistieCorrMainWin::getCaptionStr() const
{
    return tr("Mistie Correction Editor");
}
