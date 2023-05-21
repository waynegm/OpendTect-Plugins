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
#include "trigonometry.h"


uiFaultPoly::uiFaultPoly( uiParent* p )
    : uiDialog(p,uiDialog::Setup(getCaptionStr(),mNoDlgTitle,HelpKey("wgm","fpl")))
{
    setCtrlStyle( OkAndCancel );
    setOkText( tr("Create") );
    setShrinkAllowed(true);

    hor3Dfld_ = new uiIOObjSel( this, EMHorizon3DTranslatorGroup::ioContext(), uiStrings::s3DHorizon() );

    faultsfld_ = new uiFaultParSel( this, false );
    faultsfld_->attach( alignedBelow, hor3Dfld_ );

    prefixfld_ = new uiGenInput(this, tr("Fault polygon/polyline prefix"));
    prefixfld_->attach( alignedBelow, faultsfld_ );

    colorfld_ = new uiColorInput(this, uiColorInput::Setup(OD::getRandStdDrawColor()).
    lbltxt(uiStrings::sColor()) );
    colorfld_->attach(alignedBelow, prefixfld_);
}

uiFaultPoly::~uiFaultPoly()
{
    detachAllNotifiers();
//    if ( hor_->nrRefs()>0 )
//	hor_->unRef();
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

RefMan<Pick::Set> uiFaultPoly::getPolyForFault( int idx )
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
    const Geometry::FaultStickSurface* fss = fault->geometry().geometryElement();

    Geometry::BinIDSurface* surf = hor_->geometry().geometryElement();
    if (!surf) {
	uiMSG().error(tr("uiFaultPoly::getPolyForFault - failed getting BinIDSurface for horizon %1").arg(hor_->name()));
	obj->unRef();
	return nullptr;
    }

    Coord3ListImpl coords;
    for ( int idstick=0; idstick<fss->nrSticks(); idstick++ )
    {
	const TypeSet<Coord3>& stick = *(fss->getStick( idstick ));
	for (int iseg=0; iseg<stick.size()-1; iseg++ )
	{
	    Coord3 pos = lineSegmentIntersection( stick[iseg], stick[iseg+1],
						  surf );
	    if ( !mIsUdf(pos) )
		coords.add( pos );
	}
    }

    for ( int idstick=fss->nrSticks()-1; idstick>=0; idstick-- )
    {
	const TypeSet<Coord3>& stick = *(fss->getStick( idstick ));
	for (int iseg=stick.size()-1; iseg>0; iseg-- )
	{
	    Coord3 pos = lineSegmentIntersection( stick[iseg-1], stick[iseg],
						  surf );
	    if ( !mIsUdf(pos) )
		coords.add( pos );
	}
    }

    RefMan<Pick::Set> ps = new Pick::Set;
    BufferString name( prefix_ );
    name += faultname;
    ps->setName( name );
    ps->disp_.color_ = colorfld_->color();
    ps->disp_.linestyle_ = OD::LineStyle(OD::LineStyle::Solid, 1, colorfld_->color());
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

Coord3 uiFaultPoly::lineSegmentIntersection( Coord3 start, Coord3 end,
				    const Geometry::BinIDSurface* surf ) const
{
    Coord3 res = Coord3::udf();

    const StepInterval<int> rrg = surf->rowRange();
    const StepInterval<int> crg = surf->colRange();
    const BinID origin_( rrg.start, crg.start );
    const BinID step_( rrg.step, crg.step );

    const Coord spos = SI().binID2Coord().transformBackNoSnap( start );
    const Coord epos = SI().binID2Coord().transformBackNoSnap( end );

    const BinID bidstart( rrg.snap( spos.x, OD::SnapDownward ), crg.snap( spos.y, OD::SnapDownward ) );
    const BinID bidend( rrg.snap( epos.x, OD::SnapDownward ), crg.snap( epos.y, OD::SnapDownward ) );

    StepInterval<int> brrg( bidstart.inl(), bidend.inl(), rrg.step );
    brrg.sort();
    StepInterval<int> bcrg( bidstart.crl(), bidend.crl(), crg.step );
    bcrg.sort();
    for ( int row=brrg.start; row<=brrg.stop; row+=brrg.step ) {
	for ( int col=bcrg.start; col<=bcrg.stop; col+=bcrg.step ) {
	    Coord3 v00 = surf->getKnot( BinID( row, col), true );
	    Coord3 v01 = surf->getKnot( BinID( row, col+bcrg.step), true );
	    Coord3 v11 = surf->getKnot( BinID( row+brrg.step, col+bcrg.step ),
					true );
	    Coord3 v10 = surf->getKnot( BinID( row+brrg.step, col ), true );
	    res = lineSegmentIntersectsTriangle( start, end, v00, v01, v11 );
	    if ( !mIsUdf(res) )
		return res;
	    res = lineSegmentIntersectsTriangle( start, end, v00, v11, v10 );
	    if ( !mIsUdf(res) )
		return res;
	}
    }

    return res;
}
