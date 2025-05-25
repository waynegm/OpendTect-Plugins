#include "uicontourpoly.h"
/*
 *   Constant Z polyline generator class
 *   Copyright (C) 2019  Wayne Mogg
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

#include "ctxtioobj.h"
#include "emsurfacetr.h"
#include "iodir.h"
#include "iodirentry.h"
#include "uigeninput.h"
#include "uipointsetsel.h"
#include "uiseparator.h"
#include "uilabel.h"
#include "survinfo.h"
#include "uibutton.h"
#include "uimsg.h"
#include "ioobj.h"
#include "emmanager.h"
#include "emhorizon2d.h"
#include "emhorizon3d.h"
#include "survgeom2d.h"
#include "uiodmain.h"
#include "uicolor.h"
#include "randcolor.h"
#include "uistring.h"
#include "polygon.h"
#include "pickset.h"
#include "picklocation.h"
#include "picksettr.h"
#include "ptrman.h"
#include "emobject.h"


uiContourPoly::uiContourPoly( uiParent* p )
    : uiDialog(p,uiDialog::Setup(getCaptionStr(),mNoDlgTitle,HelpKey("wgm","pconz")))
{
    setCtrlStyle( OkAndCancel );
    setOkText( tr("Create") );
    setShrinkAllowed(true);

    outfld_ = uiPointSetPolygonSel::create( this, false, true, tr("Name for new polygon") );

    uiString lbl = tr("Z Value ");
    lbl.append( SI().getUiZUnitString() );
    zfld_ = new uiGenInput( this, lbl, FloatInpSpec(SI().zRange(true).start_*SI().zDomain().userFactor()) );
    zfld_->attach( rightOf, outfld_ );

    colorfld_ = new uiColorInput(this, uiColorInput::Setup(OD::getRandStdDrawColor()).
    lbltxt(uiStrings::sColor()) );
    colorfld_->attach(alignedBelow, outfld_);

}

uiContourPoly::~uiContourPoly()
{
    detachAllNotifiers();
}

bool uiContourPoly::acceptOK( CallBacker*)
{
    const IOObj* ioobj = outfld_->ioobj();
    if ( !ioobj )
	return false;

    polyname_ = ioobj->name();
    float zval = zfld_->getFValue();
    if (mIsUdf(zval)) {
	uiMSG().error( tr("Z value is undefined. Please enter a valid value") );
	return false;
    }

    z_ = zval / SI().zDomain().userFactor();
    if (!SI().zRange(false).includes(z_,false)) {
	const bool res = uiMSG().askContinue( tr("Z Value is outside survey Z range") );
	if ( !res ) return false;
    }

    return true;
}


uiString uiContourPoly::getCaptionStr() const
{
    return tr("Create Constant Z Polyline");
}

RefMan<Pick::Set> uiContourPoly::getPolygonPickSet() const
{
    RefMan<Pick::Set> ps = new Pick::Set;
    if (!ps) return nullptr;
    ps->setName( polyname_ );
    ps->disp_.color_ = colorfld_->color();
    ps->disp_.linestyle_ = OD::LineStyle(OD::LineStyle::Solid, 1, colorfld_->color());
    ps->disp_.connect_ = Pick::Set::Disp::Open;
    for ( int idx=0; idx<ps->size(); idx++ )
	ps->setZ( idx, z_ );
    return ps;
}
