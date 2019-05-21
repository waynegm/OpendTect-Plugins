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

#include "mistiecordata.h"

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
            table_->setText(RowCol(idx, lineCol), lnms.get(idx));
            table_->setValue(RowCol(idx, shiftCol), 0.0);
            table_->setValue(RowCol(idx, phaseCol), 0.0);
            table_->setValue(RowCol(idx, ampCol), 1.0);
        }
    }
}

void uiMistieCorrMainWin::openCB( CallBacker* )
{
    BufferString defseldir = FilePath(GetDataDir()).add(MistieCorrectionData::defDirStr()).fullPath();
    uiFileDialog dlg( this, true, 0, MistieCorrectionData::filtStr(), tr("Load Mistie Corrections") );
    dlg.setDirectory(defseldir);
    if (!dlg.go())
        return;

    filename_ = dlg.fileName();
    MistieCorrectionData misties;
    if (!misties.read( filename_ )) {
        filename_.setEmpty();
        ErrMsg("uiMistieCorrMainWin::openCB - error reading mistie correction file");
        return;
    }

    table_->clearTable();
    table_->setNrRows(misties.size());
    for (int idx=0; idx<misties.size(); idx++) {
        table_->setText(RowCol(idx, lineCol), misties.getDataName(idx));
        table_->setValue(RowCol(idx, shiftCol), misties.getZCor(idx));
        table_->setValue(RowCol(idx, phaseCol), misties.getPhaseCor(idx));
        table_->setValue(RowCol(idx, ampCol), misties.getAmpCor(idx));
    }
    raise();
}

void uiMistieCorrMainWin::saveCB( CallBacker* )
{
    if (filename_.isEmpty())
        saveasCB(0);
    
    MistieCorrectionData misties;
    for (int idx=0; idx<table_->nrRows(); idx++) {
        BufferString lnm(table_->text(RowCol(idx, lineCol)));
        if (!lnm.isEmpty()) 
            misties.set( lnm, table_->getFValue(RowCol(idx,shiftCol)), table_->getFValue(RowCol(idx, phaseCol)),  table_->getFValue(RowCol(idx, ampCol)) );
    }
    if (!misties.write( filename_))
        ErrMsg("uiMistieCorrMainWin::saveCB - error saving mistie corrections to file");
}

void uiMistieCorrMainWin::saveasCB( CallBacker* )
{
    BufferString defseldir = FilePath(GetDataDir()).add(MistieCorrectionData::defDirStr()).fullPath();
    uiFileDialog dlg( this, false, 0, MistieCorrectionData::filtStr(), tr("Save Mistie Corrections") );
    dlg.setMode(uiFileDialog::AnyFile);
    dlg.setDirectory(defseldir);
    dlg.setDefaultExtension(MistieCorrectionData::extStr());
    dlg.setConfirmOverwrite(true);
    dlg.setSelectedFilter(MistieCorrectionData::filtStr());
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
