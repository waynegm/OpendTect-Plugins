#include "uidehmainwin.h"

#include "bufstringset.h"
#include "uiodmain.h"
#include "uiodscenemgr.h"
#include "survinfo.h"
#include "seisioobjinfo.h"
#include "uibutton.h"
#include "ctxtioobj.h"
#include "emsurfacetr.h"
#include "iodir.h"
#include "iodirentry.h"
#include "uimsg.h"
#include "ptrman.h"
#include "emmanager.h"
#include "emhorizon3d.h"
#include "emioobjinfo.h"
#include "uiiosurface.h"
#include "uitaskrunner.h"

#include "uiseis2dlineselgrp.h"



uidehMainWin::uidehMainWin( uiParent* p )
    : uiDialog(p,uiDialog::Setup(getCaptionStr(),mNoDlgTitle,HelpKey("wgm","deh")).modal(false))
    , lineselfld_(nullptr)
    , include3Dfld_(nullptr)
{
    setCtrlStyle( OkAndCancel );
    setOkText( tr("Create") );
    setShrinkAllowed(true);
    
    uiObject* lastfld = nullptr;
    if (SI().has2D()) {
        BufferStringSet lnms;
        TypeSet<Pos::GeomID> geomids;
        SeisIOObjInfo::getLinesWithData(lnms, geomids);
        if (lnms.size() > 0 ) {
            lineselfld_ = new uiSeis2DLineSelGrp(this, OD::ChooseZeroOrMore);
            lastfld = (uiObject*) lineselfld_;
        }
    }
    
    if (SI().has3D()) {
        include3Dfld_ = new uiCheckBox(this, tr("Include 3D survey area"));
        if (lastfld!=nullptr)
            include3Dfld_->attach(alignedBelow, lastfld);
        include3Dfld_->setChecked(true);
        lastfld = (uiObject*) include3Dfld_;
    }
            
    uiSurfaceWrite::Setup swsu(EM::Horizon3D::typeStr(), EM::Horizon3D::userTypeStr());
    swsu.withsubsel(false);
    outfld_ = new uiSurfaceWrite(this, swsu);
    outfld_->attach(stretchedBelow, lastfld);
    enableSaveButton(tr("Display after create"));
}

uidehMainWin::~uidehMainWin()
{
    
}

bool uidehMainWin::acceptOK( CallBacker*)
{
    EM::IOObjInfo eminfo( outfld_->selIOObj()->key() );
    if (eminfo.isOK()) {
        uiString msg = tr("Horizon: %1\nalready exists."
                      "\nDo you wish to overwrite it?").arg(eminfo.name());
        if ( !uiMSG().askOverwrite(msg) )
            return false;
    }
    
 /*   {

    RefMan<EM::Horizon3D> hor3d = EM::Horizon3D::createWithConstZ(0.0, interpolator->getTrcKeySampling());
    if (!hor3d) {
        ErrMsg("uiGrid2D3DHorizonMainWin::acceptOK - creation of output horizon failed");
        return false;
    }
    
    {
        uiTaskRunner uitr(this);
        hor3d->setMultiID(outfld_->selIOObj()->key());
        PtrMan<Executor> saver = hor3d->saver();
        if (!saver || !TaskRunner::execute(&uitr, *saver)) {
            ErrMsg("uiGrid2D3DHorizonMainWin::acceptOK - saving output horizon failed");
            return false;
        }
    }
    if (saveButtonChecked()) {
        ODMainWin()->sceneMgr().addEMItem(EM::EMM().getObjectID(hor3d->multiID()));
        ODMainWin()->sceneMgr().updateTrees();
    }
*/
    return true;
}


uiString uidehMainWin::getCaptionStr() const
{
    return tr("Create Horizon Covering 2D-3D Data Extent");
}
