#include "uifaultpoly.h"
/*
 *   Fault Polygon/Polyline generator class
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

#include "emsurfacetr.h"
#include "uigeninput.h"
#include "uimsg.h"
#include "ioobj.h"
#include "emmanager.h"
#include "emhorizon3d.h"
#include "emfault3d.h"
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
#include "uiiosurface.h"
#include "uiioobjsel.h"
#include "faulthorintersect.h"
#include "positionlist.h"
#include "explfaultsticksurface.h"
#include "survinfo.h"
#include "survgeom3d.h"


uiFaultPoly::uiFaultPoly( uiParent* p )
    : uiDialog(p,uiDialog::Setup(getCaptionStr(),mNoDlgTitle,HelpKey("wgm","fpl")))
{
    setCtrlStyle( OkAndCancel );
    setOkText( tr("Create") );
    setShrinkAllowed(true);

    hor3Dfld_ = new uiIOObjSel( this, EMHorizon3DTranslatorGroup::ioContext(), "3D Horizon" );

    faultsfld_ = new uiFaultParSel( this, false );
    faultsfld_->attach( alignedBelow, hor3Dfld_ );
    
    prefixfld_ = new uiGenInput(this, tr("Fault polygon/polyline prefix"));
    prefixfld_->attach( alignedBelow, faultsfld_ );

    colorfld_ = new uiColorInput(this, uiColorInput::Setup(getRandStdDrawColor()).
    lbltxt(uiStrings::sColor()) );
    colorfld_->attach(alignedBelow, prefixfld_);
}

uiFaultPoly::~uiFaultPoly()
{
    detachAllNotifiers();
    hor_->unRef();
}

bool uiFaultPoly::acceptOK( CallBacker*)
{
    MultiID hor3Did;
    hor3Did.setUdf();
    const IOObj* horObj = hor3Dfld_->ioobj(true);
    if (horObj)
	hor3Did = horObj->key();
    if ( hor3Did.isUdf() ) {
	uiMSG().error(tr("Please select a 3D horizon."));
	return false;
    }
    
    EM::EMObject* obj = EM::EMM().loadIfNotFullyLoaded(hor3Did);
    if (!obj) {
	ErrMsg("uiFaultPoly::acceptOK - loading 3D horizon failed");
	return false;
    }
    obj->ref();
    mDynamicCastGet(EM::Horizon3D*,hor,obj);
    if (!hor) {
	ErrMsg("uiFaultPoly::acceptOK - casting 3D horizon failed");
	obj->unRef();
	return false;
    }
    hor_ = hor;
    
    prefix_ = prefixfld_->text();
    prefix_.trimBlanks();
    

    return true;
}

uiString uiFaultPoly::getCaptionStr() const
{
    return tr("Create Fault Polygons/Polylines");
}

int uiFaultPoly::nrFaults() const
{
    return faultsfld_->selFaultIDs().size();
}

Pick::Set* uiFaultPoly::getPolyForFault( int idx )
{
    const TypeSet<MultiID>& faultIDs = faultsfld_->selFaultIDs();
    if ( idx<0 || idx>=faultIDs.size() ) {
	uiMSG().error(tr("uiFaultPoly::getPolyForFault - fault index out of range"));
	return nullptr;
    }
    EM::EMObject* obj = EM::EMM().loadIfNotFullyLoaded(faultIDs[idx]);
    if ( !obj ) {
	uiMSG().error(tr("uiFaultPoly::getPolyForFault - failed to load fault id %1").arg(faultIDs[idx]));
	return nullptr;
    }
    obj->ref();
    mDynamicCastGet(EM::Fault3D*,fault,obj);
    if (!fault) {
	uiMSG().error(tr("uiFaultPoly::getPolyForFault - casting fault surface failed for id %1").arg(faultIDs[idx]));
	return nullptr;
    }
    
    BufferString faultname = fault->name();
    const EM::SectionID fsid = fault->sectionID( 0 );
    PtrMan<Geometry::ExplFaultStickSurface> faultsurf = new Geometry::ExplFaultStickSurface(
					fault->geometry().sectionGeometry(fsid), SI().zScale() );
    faultsurf->setCoordList( new Coord3ListImpl, new Coord3ListImpl );
    if ( !faultsurf->update(true,0) ) {
	uiMSG().error(tr("uiFaultPoly::getPolyForFault - can't update fault surface for fault %1").arg(faultname));
	obj->unRef();
	return nullptr;
    }
    
    const EM::SectionID hsid = hor_->sectionID( 0 );
    Geometry::BinIDSurface* surf = hor_->geometry().sectionGeometry(hsid);
    if (!surf) {
	uiMSG().error(tr("uiFaultPoly::getPolyForFault - failed getting BinIDSurface for horizon %1").arg(hor_->name()));
	obj->unRef();
	return nullptr;
    }
    
    Coord3ListImpl coords;
    Geometry::FaultBinIDSurfaceIntersector fsi( 0.0, *surf, *faultsurf, coords );
    fsi.compute();
    
    Pick::Set* ps = new Pick::Set;
    BufferString name( prefix_ );
    name += faultname;
    ps->setName( name );
    ps->disp_.color_ = colorfld_->color();
    ps->disp_.connect_ = Pick::Set::Disp::Open;
    if ( coords.size()>0 ) {
	int nextid = coords.nextID( -1 );
	while ( nextid!=-1 ) {
	    if ( coords.isDefined(nextid) ) {
		Pick::Location loc(coords.get(nextid));
		ps->add( loc );
	    }
	    nextid = coords.nextID( nextid );
	}
    }
    return ps;
}

bool uiFaultPoly::addToDisplay() const
{
    return saveButtonChecked();
}

