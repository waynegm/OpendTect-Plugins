#include "uicorrviewer.h"

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

uiCorrViewer::uiCorrViewer( uiParent* p, const MistieCorrectionData& corrs )
    : uiDialog(p, uiDialog::Setup(getCaptionStr(),mNoDlgTitle,mNoHelpKey))
    , saveitem_(uiStrings::sSave(), "save", "", mCB(this,uiCorrViewer,saveCB), sMnuID++)
    , saveasitem_(uiStrings::sSaveAs(), "saveas", "", mCB(this,uiCorrViewer,saveasCB), sMnuID++)
    , corrs_(corrs)
{
    createToolBar();

    table_ = new uiTable( this, uiTable::Setup().rowgrow(false).selmode(uiTable::NoSelection),"Mistie Correction Table" );
    uiStringSet lbls;
    lbls.add(tr("Line/Dataset")).add(tr("Z Shift (%1)").arg(SI().getZUnitString(false)))
        .add(tr("Phase Rotation (deg)")).add(tr("Amplitude Scalar"));

    table_->setColumnLabels( lbls);
    table_->showGrid( true );
    table_->setLeftHeaderHidden( true );
    table_->setPrefWidthInChars(50);
    table_->setPrefHeightInRows(50);
    table_->setTableReadOnly(true);

    fillTable();
    setCtrlStyle(CloseOnly);
    setModal(false);
    mAttachCB(windowClosed, uiCorrViewer::closeCB);
}

uiCorrViewer::~uiCorrViewer()
{
    detachAllNotifiers();
}

void uiCorrViewer::newCorrData()
{
    filename_.setEmpty();
    fillTable();
}

void uiCorrViewer::createToolBar()
{
    tb_ = new uiToolBar( this, tr("Mistie Correction Viewer") );
    tb_->addButton( saveitem_ );
    tb_->addButton( saveasitem_ );
}

void uiCorrViewer::fillTable()
{
    table_->clearTable();
    table_->setNrRows(corrs_.size());

    BufferString line;
    float zcor, phasecor, ampcor;
    for (int idx=0; idx<corrs_.size(); idx++) {
        line = corrs_.getDataName(idx);
        zcor = corrs_.getZCor(idx);
        phasecor = corrs_.getPhaseCor(idx);
        ampcor = corrs_.getAmpCor(idx);
        table_->setText(RowCol(idx, lineCol), line);
        table_->setValue(RowCol(idx, shiftCol), zcor, 2);
        table_->setValue(RowCol(idx, phaseCol), phasecor, 2);
        table_->setValue(RowCol(idx, ampCol), ampcor, 2);
    }
}

bool uiCorrViewer::checkSave()
{
    if (corrs_.size()>0 && filename_.isEmpty()) {
        uiString msg = tr("The latest mistie corrections have not been saved.\nDo you want to save them?");
        int result = uiMSG().askSave(msg,false);
        if (result == 1)
            saveasCB(0);
    }
    return true;
}

void uiCorrViewer::closeCB( CallBacker* )
{
    if (!checkSave())
        return;
}

void uiCorrViewer::saveCB( CallBacker* )
{
    if (filename_.isEmpty())
        saveasCB(0);

    if (!corrs_.write( filename_))
        ErrMsg("uiCorrViewer::saveCB - error saving mistie corrections to file");
}

void uiCorrViewer::saveasCB( CallBacker* )
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

uiString uiCorrViewer::getCaptionStr() const
{
    return tr("Mistie Correction Viewer");
}
