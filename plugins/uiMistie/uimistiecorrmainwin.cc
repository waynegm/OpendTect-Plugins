#include "uimistiecorrmainwin.h"

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

#include "mistiecordata.h"

static int sMnuID = 0;


class uiMergeCorDlg : public uiDialog
{ mODTextTranslationClass(uiMergeCorDlg);
public:
    uiMergeCorDlg( uiParent *p )
    : uiDialog(p,Setup(tr("Merge Other Correction File"),mNoDlgTitle,mTODOHelpKey))
    {
        BufferString defseldir = FilePath(GetDataDir()).add(MistieCorrectionData::defDirStr()).fullPath();
        filefld_ = new uiFileInput( this, tr("Mistie Correction File"), uiFileInput::Setup(uiFileDialog::Gen)
                                                                    .forread(true)
                                                                    .defseldir(defseldir)
                                                                    .filter(MistieCorrectionData::filtStr()) );
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

uiMistieCorrMainWin::uiMistieCorrMainWin( uiParent* p )
    : uiMainWin(p, getCaptionStr() )
    , newitem_(uiStrings::sNew(), "new", "", mCB(this,uiMistieCorrMainWin,newCB), sMnuID++)
    , openitem_(uiStrings::sOpen(), "open", "", mCB(this,uiMistieCorrMainWin,openCB), sMnuID++)
    , saveitem_(uiStrings::sSave(), "save", "", mCB(this,uiMistieCorrMainWin,saveCB), sMnuID++)
    , saveasitem_(uiStrings::sSaveAs(), "saveas", "", mCB(this,uiMistieCorrMainWin,saveasCB), sMnuID++)
    , mergeitem_(uiStrings::sMerge(), "plus", "", mCB(this,uiMistieCorrMainWin,mergeCB), sMnuID++)
    , helpitem_(tr("Help"), "contexthelp", "", mCB(this,uiMistieCorrMainWin,helpCB), sMnuID++)

{
    createToolBar();

    table_ = new uiTable( this, uiTable::Setup().rowgrow(true)
                                                .selmode(uiTable::Multi),
                          "Mistie Correction Table" );
    uiStringSet lbls;
    lbls.add(tr("Line/Dataset")).add(tr("Z Shift (%1)").arg(SI().getZUnitString(false)))
        .add(tr("Phase Rotation (deg)")).add(tr("Amplitude Scalar"));

    table_->setColumnLabels( lbls);
    table_->showGrid( true );
    table_->setLeftHeaderHidden( true );
    table_->setPrefWidthInChar(50);
    table_->setPrefHeightInRows(50);
    mAttachCB(table_->rowInserted, uiMistieCorrMainWin::newrowCB);

    locknames_ = new uiCheckBox(this, tr("Lock line/dataset column"));
    locknames_->attach(alignedBelow, table_);
    locknames_->setChecked(true);
    mAttachCB(locknames_->activated, uiMistieCorrMainWin::locknamesCB);

    newCB(0);
    locknamesCB(0);
}

uiMistieCorrMainWin::~uiMistieCorrMainWin()
{
    detachAllNotifiers();
}

void uiMistieCorrMainWin::createToolBar()
{
    tb_ = new uiToolBar( this, tr("Mistie Correction Editor") );
    tb_->addButton( newitem_ );
    tb_->addButton( openitem_ );
    tb_->addButton( saveitem_ );
    tb_->addButton( saveasitem_ );
    tb_->addSeparator();
    tb_->addButton( mergeitem_ );
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

void uiMistieCorrMainWin::locknamesCB( CallBacker* )
{
    table_->setColumnReadOnly(lineCol, locknames_->isChecked());
}

void uiMistieCorrMainWin::mergeCB( CallBacker* )
{
    uiMergeCorDlg dlg(this);
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
            table_->setValue(RowCol(row[irow], shiftCol), merge.getZCor(idx), 2);
            table_->setValue(RowCol(row[irow], phaseCol), merge.getPhaseCor(idx), 2);
            table_->setValue(RowCol(row[irow], ampCol), merge.getAmpCor(idx), 2);
        } else {
            int newRow = table_->nrRows();
            table_->insertRows(newRow, 1);
            table_->setText(RowCol(newRow, lineCol), merge.getDataName(idx));
            table_->setValue(RowCol(newRow, shiftCol), merge.getZCor(idx), 2);
            table_->setValue(RowCol(newRow, phaseCol), merge.getPhaseCor(idx), 2);
            table_->setValue(RowCol(newRow, ampCol), merge.getAmpCor(idx), 2);
        }
    }
    raise();
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
            table_->setValue(RowCol(idx, shiftCol), 0.0, 2);
            table_->setValue(RowCol(idx, phaseCol), 0.0, 2);
            table_->setValue(RowCol(idx, ampCol), 1.0, 2);
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
        table_->setValue(RowCol(idx, shiftCol), misties.getZCor(idx), 2);
        table_->setValue(RowCol(idx, phaseCol), misties.getPhaseCor(idx), 2);
        table_->setValue(RowCol(idx, ampCol), misties.getAmpCor(idx), 2);
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
