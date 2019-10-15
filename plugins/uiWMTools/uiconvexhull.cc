#include "uiconvexhull.h"
/*
 *   Convex hull generator class
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

#include "ui2d3ddataselgrp.h"
#include "uihorinputgrp.h"



uiConvexHull::uiConvexHull( uiParent* p )
    : uiDialog(p,uiDialog::Setup(getCaptionStr(),mNoDlgTitle,HelpKey("wgm","pcvx")))
    , usehorfld_(nullptr)
    , horinputfld_(nullptr)
{
    setCtrlStyle( OkAndCancel );
    setOkText( tr("Create") );
    setShrinkAllowed(true);
    
    namefld_ = new uiGenInput(this, tr("Name for new polygon"));
    
    uiString lbl = tr("Z Value ");
    lbl.append( SI().getUiZUnitString() );
    zfld_ = new uiGenInput( this, lbl, FloatInpSpec(SI().zRange(true).start*SI().zDomain().userFactor()) );
    zfld_->attach( rightOf, namefld_ ); 
    
    colorfld_ = new uiColorInput(this, uiColorInput::Setup(getRandStdDrawColor()).
    lbltxt(uiStrings::sColor()) );
    colorfld_->attach(alignedBelow, namefld_);

    uiSeparator* navsep = new uiSeparator(this, "Navigation Data Selection");
    navsep->attach(stretchedBelow, colorfld_);
    
    usenavfld_ = new uiCheckBox(this, tr("Use Navigation Data"));
    usenavfld_->attach(alignedBelow, colorfld_);
    usenavfld_->attach(ensureBelow, navsep);
    usenavfld_->setChecked(true);
    mAttachCB(usenavfld_->activated, uiConvexHull::usenavCB);
    
    navinputgrp_ = new WMLib::ui2D3DDataSelGrp( this );
    navinputgrp_->attach(alignedBelow, usenavfld_);
    
    navsep = new uiSeparator(this, "Horizon Data Selection");
    navsep->attach(stretchedBelow, navinputgrp_);
    
    usehorfld_ = new uiCheckBox(this, tr("Use Horizon Data"));
    usehorfld_->attach(alignedBelow, navinputgrp_);
    usehorfld_->attach(ensureBelow, navsep);
    usehorfld_->setChecked(!usenavfld_->isChecked());
    mAttachCB(usehorfld_->activated, uiConvexHull::usehorCB);
   
    {
	CtxtIOObj ctio2D(EMHorizon2DTranslatorGroup::ioContext());
	const IODir iodir2D( ctio2D.ctxt_.getSelKey() );
	const IODirEntryList entries2D( iodir2D, ctio2D.ctxt_ );
	bool has2Dhorizon = SI().has2D() && entries2D.size()>0;
	
	CtxtIOObj ctio3D(EMHorizon3DTranslatorGroup::ioContext());
	const IODir iodir3D( ctio3D.ctxt_.getSelKey() );
	const IODirEntryList entries3D( iodir3D, ctio3D.ctxt_ );
	bool has3Dhorizon = SI().has3D() && entries3D.size()>0;
	
	if (has2Dhorizon || has3Dhorizon) {
	    navsep = new uiSeparator(this, "Horizon Data Selection");
	    navsep->attach(stretchedBelow, navinputgrp_);
	    
	    usehorfld_ = new uiCheckBox(this, tr("Use Horizon Data"));
	    usehorfld_->attach(alignedBelow, navinputgrp_);
	    usehorfld_->attach(ensureBelow, navsep);
	    usehorfld_->setChecked(!usenavfld_->isChecked());
	    mAttachCB(usehorfld_->activated, uiConvexHull::usehorCB);
	    
	    horinputfld_ = new WMLib::uiHorInputGrp( this, has2Dhorizon, has3Dhorizon );
	    horinputfld_->attach(alignedBelow, usehorfld_);
	}
    }
    
    mAttachCB( postFinalise(), uiConvexHull::initGrp );
}

uiConvexHull::~uiConvexHull()
{
    detachAllNotifiers();
}

bool uiConvexHull::acceptOK( CallBacker*)
{
    polyname_ = namefld_->text();
    polyname_.trimBlanks();
    if ( polyname_.isEmpty() ) {
	uiMSG().error( tr("Please specify a name for the polygon") );
	return false;
    }
    
    PtrMan<CtxtIOObj> ctio = mMkCtxtIOObj(PickSet);
    ctio->setName( polyname_ );
    const IODir iodir( ctio->ctxt_.getSelKey() );
    if ( iodir.get( polyname_, ctio->ctxt_.trgroup_->groupName() ) ) {
	uiString msg = tr("Overwrite existing '%1'?").arg(polyname_);
	if ( !uiMSG().askOverwrite(msg) )
	    return false;
    }
    
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
    
    
    if ( usenavfld_->isChecked() )
	fillPolyFromNav();
    else
	fillPolyFromHorizon();
    
    poly_.convexHull();

    return true;
}

void uiConvexHull::initGrp(CallBacker*)
{
    dataselCB(0);
}

void uiConvexHull::usenavCB( CallBacker*)
{
    if ( usehorfld_ )
	usehorfld_->setChecked(!usenavfld_->isChecked());
    dataselCB(0);
}

void uiConvexHull::usehorCB( CallBacker*)
{
    if ( usehorfld_ )
	usenavfld_->setChecked(!usehorfld_->isChecked());
    dataselCB(0);
}

void uiConvexHull::dataselCB(CallBacker*)
{
    navinputgrp_->setSensitive( usenavfld_->isChecked() );
    if ( horinputfld_ )
	horinputfld_->setSensitive( usehorfld_->isChecked() );
}

uiString uiConvexHull::getCaptionStr() const
{
    return tr("Create Convex Hull Polygon");
}

void uiConvexHull::fillPolyFromNav()
{
    TypeSet<Pos::GeomID> geomids;
    navinputgrp_->get2DGeomIDs(geomids);
    poly_.setEmpty();
    
    for (int idx=0; idx<geomids.size(); idx++) {
	mDynamicCastGet( const Survey::Geometry2D*, geom2d, Survey::GM().getGeometry(geomids[idx]) );
	if (!geom2d)
	    continue;
	const PosInfo::Line2DData& geom = geom2d->data();
	const TypeSet<PosInfo::Line2DPos>& posns = geom.positions();
	poly_.add( posns[0].coord_ );
	poly_.add( posns[posns.size()-1].coord_ );
    }

    TrcKeyZSampling tkz;
    navinputgrp_->get3Dsel( tkz );
    TrcKeySampling tk = tkz.hsamp_;
    if ( !tkz.isEmpty() ) {
	poly_.add( SI().transform( BinID(tk.start_.inl(), tk.start_.crl()) ) );
	poly_.add( SI().transform( BinID(tk.start_.inl(), tk.stop_.crl()) ) );
	poly_.add( SI().transform( BinID(tk.stop_.inl(), tk.stop_.crl()) ) );
	poly_.add( SI().transform( BinID(tk.stop_.inl(), tk.start_.crl()) ) );
    }
}

void uiConvexHull::fillPolyFromHorizon()
{
    poly_.setEmpty();
    MultiID hor2Did, hor3Did;
    horinputfld_->getHorIds(hor2Did, hor3Did);
    if (!hor3Did.isUdf()) {
	TrcKeyZSampling tkzs;
	horinputfld_->get3Dsel( tkzs );
	EM::EMObject* obj = EM::EMM().loadIfNotFullyLoaded(hor3Did);
	if (!obj) {
	    ErrMsg("uiConvexHull::fillPolyFromHorizon - loading 3D horizon failed");
	    return;
	}
	obj->ref();
	mDynamicCastGet(EM::Horizon3D*,hor,obj);
	if (!hor) {
	    ErrMsg("uiConvexHull::fillPolyFromHorizon - casting 3D horizon failed");
	    obj->unRef();
	    return;
	}
	Pick::Set ps;
	hor->geometry().getBoundingPolygon( 0, ps );
	ps.getPolygon( poly_ );
	obj->unRef();
    }
    
    if ( !hor2Did.isUdf()) {
	TypeSet<Pos::GeomID> mids;
	horinputfld_->getGeoMids(mids);
	EM::EMObject* obj = EM::EMM().loadIfNotFullyLoaded(hor2Did);
	if (!obj) {
	    ErrMsg("uiConvexHull::fillPolyFromHorizon - loading 2D horizon failed");
	    return;
	}
	obj->ref();
	mDynamicCastGet(EM::Horizon2D*,hor,obj);
	if (!hor) {
	    ErrMsg("uiConvexHull - casting 2D horizon failed");
	    obj->unRef();
	    return;
	}
	for (int idx=0; idx<mids.size(); idx++) {
	    const StepInterval<int> trcrg = hor->geometry().colRange( mids[idx] );
	    mDynamicCastGet(const Survey::Geometry2D*,survgeom2d,Survey::GM().getGeometry(mids[idx]))
	    if (!survgeom2d || trcrg.isUdf() || !trcrg.step)
		continue;
	    
	    TrcKey tk( mids[idx], -1 );
	    int spnr = mUdf(int);
	    Coord coord;
	    bool first = true;
	    for ( int trcnr=trcrg.start; trcnr<=trcrg.stop; trcnr+=trcrg.step ) {
		tk.setTrcNr( trcnr );
		const float z = hor->getZ( tk );
		if (mIsUdf(z))
		    continue;
		
		survgeom2d->getPosByTrcNr( trcnr, coord, spnr );
		if (first) {
		    poly_.add(coord);
		    first = false;
		}
	    }
	    poly_.add(coord);
	}
	obj->unRef();
    }
}

bool uiConvexHull::addToDisplay() const
{
    return saveButtonChecked();
}

Pick::Set* uiConvexHull::getPolygonPickSet() const
{
    Pick::Set* ps = new Pick::Set;
    ps->setName( polyname_ );
    ps->disp_.color_ = colorfld_->color();
    ps->disp_.connect_ = Pick::Set::Disp::Close;
    for (int iv=0; iv<poly_.size(); iv++) {
	Coord pos = poly_.getVertex( iv );
	ps->add( Pick::Location(pos.x, pos.y, z_) );
    }
    return ps;
}
