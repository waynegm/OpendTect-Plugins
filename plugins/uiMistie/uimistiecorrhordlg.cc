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

#include "uihorinputgrp.h"



uiMistieCorrHorDlg::uiMistieCorrHorDlg( uiParent* p )
    : uiDialog(p,uiDialog::Setup(getCaptionStr(),mNoDlgTitle,HelpKey("wgm","mistie")).modal(false))
{
    setCtrlStyle( OkAndCancel );
    setOkText( tr("Apply") );
    setShrinkAllowed(true);

    CtxtIOObj ctio2D(EMHorizon2DTranslatorGroup::ioContext());
    const IODir iodir2D( ctio2D.ctxt_.getSelKey() );
    const IODirEntryList entries2D( iodir2D, ctio2D.ctxt_ );
    bool has2Dhorizon = SI().has2D() && entries2D.size()>0;

    CtxtIOObj ctio3D(EMHorizon3DTranslatorGroup::ioContext());
    const IODir iodir3D( ctio3D.ctxt_.getSelKey() );
    const IODirEntryList entries3D( iodir3D, ctio3D.ctxt_ );
    bool has3Dhorizon = SI().has3D() && entries3D.size()>0;

    horinpgrp_ = new WMLib::uiHorInputGrp(this, has2Dhorizon, has3Dhorizon, false);

    
}

uiMistieCorrHorDlg::~uiMistieCorrHorDlg()
{
    detachAllNotifiers();
}

bool uiMistieCorrHorDlg::acceptOK( CallBacker*)
{

    return true;
}


uiString uiMistieCorrHorDlg::getCaptionStr() const
{
    return tr("Apply Mistie Corrections to Horizon");
}
