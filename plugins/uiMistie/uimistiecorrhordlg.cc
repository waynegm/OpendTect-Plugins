#include "uimistiecorrhordlg.h"
/*
 *   Apply Mistie Corrections to Horizon Interpretation
 *   Copyright (C) 2020  Wayne Mogg
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "uigeninput.h"
#include "uiseparator.h"
#include "uilabel.h"
#include "bufstringset.h"
#include "survinfo.h"
#include "seisioobjinfo.h"
#include "uibutton.h"
#include "uimsg.h"
#include "ioobj.h"
#include "emmanager.h"
#include "emioobjinfo.h"
#include "emhorizon3d.h"
#include "emhorizon2d.h"
#include "uiiosurface.h"
#include "survgeom2d.h"
#include "uitaskrunner.h"
#include "uiodmain.h"
#include "uiodscenemgr.h"
#include "iodir.h"
#include "filepath.h"
#include "iodirentry.h"
#include "ctxtioobj.h"
#include "emsurfacetr.h"
#include "oddirs.h"
#include "mistiecordata.h"
#include "mistieapplytohorizon.h"
#include "uifileinput.h"
#include "survinfo.h"
#include "string.h"

#include "uihorinputgrp.h"



uiMistieCorrHorDlg::uiMistieCorrHorDlg( uiParent* p, bool for2D )
    : uiDialog(p,uiDialog::Setup(getCaptionStr(),mNoDlgTitle,HelpKey("wgm","mistie")).modal(false))
{
    setCtrlStyle( OkAndCancel );
    setOkText( tr("Apply") );
    setShrinkAllowed(true);

    CtxtIOObj ctio2D(EMHorizon2DTranslatorGroup::ioContext());
    const IODir iodir2D( ctio2D.ctxt_.getSelKey() );
    const IODirEntryList entries2D( iodir2D, ctio2D.ctxt_ );
    bool has2Dhorizon = SI().has2D() && entries2D.size()>0;

    BufferString defseldir = FilePath(GetDataDir()).add(MistieCorrectionData::defDirStr()).fullPath();
    mistiefilefld_ = new uiFileInput( this, tr("Mistie Database"),
				      uiFileInput::Setup(uiFileDialog::Gen)
				      .forread(true).defseldir(defseldir)
				      .filter(MistieCorrectionData::filtStr()) );

    if ( for2D && has2Dhorizon )
	horinpgrp_ = new WMLib::uiHorInputGrp(this, true, false, false);
    else
	horinpgrp_ = new WMLib::uiHorInputGrp(this, false, true, false);

    horinpgrp_->attach(alignedBelow, mistiefilefld_);
    if ( for2D && has2Dhorizon ) {
	uiSurfaceWrite::Setup swsu(EM::Horizon2D::typeStr(), EM::Horizon2D::userTypeStr());
	swsu.withsubsel(false);
	outfld_ = new uiSurfaceWrite(this, swsu);
    } else {
	uiSurfaceWrite::Setup swsu(EM::Horizon3D::typeStr(), EM::Horizon3D::userTypeStr());
	swsu.withsubsel(false);
	outfld_ = new uiSurfaceWrite(this, swsu);
    }
    outfld_->attach(alignedBelow, horinpgrp_);
}

uiMistieCorrHorDlg::~uiMistieCorrHorDlg()
{
    detachAllNotifiers();
}

bool uiMistieCorrHorDlg::acceptOK( CallBacker*)
{
    if (!strlen(mistiefilefld_->fileName())) {
	uiMSG().error( tr("Please specify an input mistie correction file to apply") );
	return false;
    }
    MultiID hor2Did, hor3Did;
    horinpgrp_->getHorIds(hor2Did, hor3Did);
    if (!hor2Did.isUdf()) {
	if (horinpgrp_->num2DLinesChosen()==0) {
	    uiMSG().error( tr("Please select the 2D line(s) to correct") );
	    return false;
	}
	const MultiID newhorID = outfld_->selIOObj()->key();
	EM::IOObjInfo eminfo( newhorID );
	if (eminfo.isOK()) {
	    uiString msg = tr("Horizon: %1\nalready exists."
	    "\nDo you wish to overwrite it?").arg(eminfo.name());
	    if ( !uiMSG().askOverwrite(msg) )
		return false;
	}
	TypeSet<Pos::GeomID> geomids;
	horinpgrp_->getGeoMids(geomids);
	uiTaskRunner uitr(this);
	MistieApplyToHorizon2D mistieApplier(hor2Did, newhorID, geomids, mistiefilefld_->fileName());
	return TaskRunner::execute(&uitr, mistieApplier);
    }

    if (!hor3Did.isUdf()) {
	const MultiID newhorID = outfld_->selIOObj()->key();
	EM::IOObjInfo eminfo( newhorID );
	if (eminfo.isOK()) {
	    uiString msg = tr("Horizon: %1\nalready exists."
	    "\nDo you wish to overwrite it?").arg(eminfo.name());
	    if ( !uiMSG().askOverwrite(msg) )
		return false;
	}
	uiTaskRunner uitr(this);
	MistieApplyToHorizon3D mistieApplier(hor3Did, newhorID, mistiefilefld_->fileName());
	return TaskRunner::execute(&uitr, mistieApplier);
    }

    return true;
}


uiString uiMistieCorrHorDlg::getCaptionStr() const
{
    return tr("Apply Mistie Corrections to Horizon");
}
