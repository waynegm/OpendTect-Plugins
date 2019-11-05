#include "uigridgrp.h"
#include "ui3drangegrp.h"

#include "survinfo.h"
#include "uigeninput.h"
#include "uiioobjsel.h"
#include "uiiosurface.h"
#include "uistring.h"
#include "ctxtioobj.h"
#include "pickset.h"
#include "picksettr.h"
#include "bufstringset.h"
#include "uimsg.h"
#include "emioobjinfo.h"
#include "emhorizon3d.h"
#include "wmgridder2d.h"
#include "uipolygonparsel.h"
#include "uicompoundparsel.h"

ui2D3DInterpol* ui2D3DInterpol::create( const char* methodName, uiParent* p )
{
    BufferString tmp(methodName);
    if (tmp == wmGridder2D::MethodNames[wmGridder2D::LTPS])
	return (ui2D3DInterpol*) new uiLTPS(p);
    else if (tmp == wmGridder2D::MethodNames[wmGridder2D::IDW])
        return (ui2D3DInterpol*) new uiIDW(p);
    else if (tmp == wmGridder2D::MethodNames[wmGridder2D::MBA])
	return (ui2D3DInterpol*) new uiMBA(p);
    else if (tmp == wmGridder2D::MethodNames[wmGridder2D::NRN])
	return (ui2D3DInterpol*) new uiNearestNeighbour(p);
    else {
        ErrMsg("ui2D3DInterpol::create - unrecognised method name");
        return nullptr;
    }
}

uiGridGrp::uiGridGrp( uiParent* p )
: uiDlgGroup(p, tr("Gridding Parameters"))
{
    scopefld_ = new uiGenInput(this, tr("Grid Extent"), StringListInpSpec(wmGridder2D::ScopeNames));
    scopefld_->setValue( wmGridder2D::ConvexHull );
    scopefld_->valuechanged.notify( mCB(this,uiGridGrp,scopeChgCB));
    
    uiSurfaceRead::Setup swsu(EM::Horizon3D::typeStr());
    swsu.withsubsel(false).withattribfld(false).withsectionfld(false);
    horfld_ = new uiSurfaceRead(this, swsu);
    horfld_->getObjSel()->setLabelText(uiString::emptyString());
    horfld_->attach( rightOf, scopefld_ );
    horfld_->inpChange.notify( mCB(this,uiGridGrp,horChgCB));
    
    gridfld_ = new WMLib::ui3DRangeGrp(this, tr("Grid Area"), false);
    gridfld_->setSensitive(false, true);
    gridfld_->attach(alignedBelow, scopefld_);
    gridfld_->setTrcKeySampling( SI().sampling(true).hsamp_ );
    
    methodfld_ = new uiGenInput( this, tr("Algorithm"),StringListInpSpec(wmGridder2D::MethodNames) );
    methodfld_->attach( alignedBelow, gridfld_ );
    methodfld_->valuechanged.notify( mCB(this,uiGridGrp,methodChgCB) );
    methodfld_->setValue(wmGridder2D::LTPS);
    
    polycropfld_ = new WMLib::uiPolygonParSel(this, tr("Cropping Polygon"), false);
    polycropfld_->attach( alignedBelow, methodfld_ );
    
    faultpolyfld_ = new WMLib::uiPolygonParSel(this, tr("Fault Polygons"), true);
    faultpolyfld_->attach( alignedBelow, polycropfld_ );

//	faultsurffld_ = new uiFaultParSel( this, false );
//	faultsurffld_->attach( alignedBelow, faultpolyfld_);
    
    for ( int idx=0; wmGridder2D::MethodNames[idx]; idx++ )
    {
	ui2D3DInterpol* methodgrp = ui2D3DInterpol::create( wmGridder2D::MethodNames[idx], this );
	if ( methodgrp )
	    methodgrp->attach( alignedBelow, faultpolyfld_ );
	methodgrps_ += methodgrp;
    }

    setHAlignObj( methodfld_ );
    update();
}

uiGridGrp::~uiGridGrp()
{
    detachAllNotifiers();
}

void uiGridGrp::update()
{
    methodChgCB(0);
    scopeChgCB(0);
}

void uiGridGrp::scopeChgCB(CallBacker* )
{
    if (scopefld_->getIntValue() == wmGridder2D::BoundingBox) {
        horfld_->display(false);
        gridfld_->displayFields(false, true);
        gridfld_->setSensitive(false, true);
	gridfld_->setSnap(true);
    } else if (scopefld_->getIntValue() == wmGridder2D::ConvexHull) {
	horfld_->display(false);
	gridfld_->displayFields(false, true);
	gridfld_->setSensitive(false, true);
	gridfld_->setSnap(true);
    } else if (scopefld_->getIntValue() == wmGridder2D::Horizon) {
        horfld_->display(true);
        gridfld_->setSensitive(false, false);
        gridfld_->displayFields(true, true);
	gridfld_->setSnap(false);
	horChgCB(0);
    } else if (scopefld_->getIntValue() == wmGridder2D::Range) {
	horfld_->display(false);
	gridfld_->setSensitive(true, true);
	gridfld_->displayFields(true, true);
	gridfld_->setSnap(true);
    }
}

void uiGridGrp::horChgCB(CallBacker*)
{
    const IOObj* horObj = horfld_->selIOObj();
    if (horObj && scopefld_->getIntValue() == wmGridder2D::Horizon) {
        EM::IOObjInfo eminfo(horObj->key());
        TrcKeySampling hs;
        hs.set(eminfo.getInlRange(), eminfo.getCrlRange());
        gridfld_->setTrcKeySampling( hs );
    }
}

void uiGridGrp::methodChgCB(CallBacker* )
{
    const int selidx = methodfld_ ? methodfld_->getIntValue( 0 ) : 0;
    for ( int idx=0; idx<methodgrps_.size(); idx++ )
    {
        if ( methodgrps_[idx] )
            methodgrps_[idx]->display( idx==selidx );
    }
    faultpolyfld_->display(wmGridder2D::canHandleFaultPolygons( wmGridder2D::MethodNames[selidx]));
//    faultsurffld_->display(wmGridder2D::canHandleFaultSurfaces( wmGridder2D::MethodNames[selidx]));
}

bool uiGridGrp::fillPar( IOPar& par ) const
{
    const int scope = scopefld_->getIntValue();
    par.set (wmGridder2D::sKeyScopeType(), scope );

    if ( scope == wmGridder2D::Horizon ) {
        const IOObj* horObj = horfld_->selIOObj();
        if (horObj)
            par.set( wmGridder2D::sKeyScopeHorID(), horObj->key() ); 
    }
    
    const TypeSet<MultiID>& croppolytids = polycropfld_->selPolygonIDs(); 
    if (croppolytids.size()==1)
        par.set( wmGridder2D::sKeyClipPolyID(), croppolytids[0] );

    const TypeSet<MultiID>& selpolytids = faultpolyfld_->selPolygonIDs();
    par.set( wmGridder2D::sKeyFaultPolyNr(), selpolytids.size() );
    for ( int idx=0; idx<selpolytids.size(); idx++ )
        par.set( IOPar::compKey(wmGridder2D::sKeyFaultPolyID(),idx), selpolytids[idx] );
    
//    const TypeSet<MultiID>& selfaultids = fltselfld_->selFaultIDs();
//    par.set( wmGridder2D::sKeyFaultNr(), selfaultids.size() );
//    for ( int idx=0; idx<selfaultids.size(); idx++ )
//        par.set( IOPar::compKey(wmGridder2D::sKeyFaultID(),idx), selfaultids[idx] );

    const BinID step = gridfld_->getTrcKeySampling().step_;
    if ( step.inl() <= 0 || step.crl() <= 0 )
    {
        uiMSG().error(
            uiStrings::phrEnter(tr("positive integer value for steps")) );
        return false;
    }
    gridfld_->fillPar( par );
    
    const int methodidx = methodfld_->getIntValue( 0 );
    par.set( wmGridder2D::sKeyMethod(), wmGridder2D::MethodNames[methodidx] );
    return methodgrps_[methodidx]->fillPar( par );
}

void uiGridGrp::usePar( const IOPar& par )
{
    int scope = 0;
    if (par.get(wmGridder2D::sKeyScopeType(), scope))
        scopefld_->setValue(scope);
    
    MultiID horID;
    if (par.get(wmGridder2D::sKeyScopeHorID(), horID))
        horfld_->setInput(horID);
    
    MultiID clipPolyID;
    polycropfld_->setEmpty();
    if (par.get(wmGridder2D::sKeyClipPolyID(), clipPolyID)) {
        TypeSet<MultiID> polyIDs;
        polyIDs += clipPolyID;
        polycropfld_->setSelectedPolygons(polyIDs);
    }
    
    int nrfaultpoly = 0;
    faultpolyfld_->setEmpty();
    if (par.get(wmGridder2D::sKeyFaultPolyNr(), nrfaultpoly)) {
        if (nrfaultpoly>0) {
            TypeSet<MultiID> polyIDs;
            for (int idx=0; idx<nrfaultpoly; idx++) {
                MultiID id;
                if (!par.get(IOPar::compKey(wmGridder2D::sKeyFaultPolyID(), idx), id))
                    return;
                polyIDs += id;
            }
            faultpolyfld_->setSelectedPolygons(polyIDs);
        }
    }
    
    gridfld_->usePar(par);
    
    BufferStringSet strs( wmGridder2D::MethodNames );
    int methodidx = strs.indexOf(par.find(wmGridder2D::sKeyMethod()));
    methodfld_->setValue( methodidx );
    methodgrps_[methodidx]->usePar( par );
            
    update();
}

ui2D3DInterpol::ui2D3DInterpol( uiParent* p )
: uiGroup(p)
{}

uiIDW::uiIDW( uiParent* p )
    : ui2D3DInterpol(p)
{
    uiString titletext( tr("Search radius %1").arg(SI().getUiXYUnitString()) );
    searchradiusfld_ = new uiGenInput( this, titletext, FloatInpSpec(8000.0) );
    searchradiusfld_->setWithCheck( true );
    searchradiusfld_->setChecked( true );

    maxpointsfld_ = new uiGenInput( this, tr("Maximum points"), IntInpSpec(50) );
    maxpointsfld_->setWithCheck( true );
    maxpointsfld_->setChecked( true );
    maxpointsfld_->attach(alignedBelow, searchradiusfld_);
}

bool uiIDW::fillPar( IOPar& par ) const
{
    if ( searchradiusfld_->isChecked() ) {
	const float radius = searchradiusfld_->getFValue(0);
	if ( radius<=0 )
	{
	    uiMSG().error( "Search radius must be positive" );
	    return false;
	}
	par.set( wmGridder2D::sKeySearchRadius(), radius );
    }

    if ( maxpointsfld_->isChecked() ) {
	const int npoints = maxpointsfld_->getIntValue(0);
	if ( npoints<=0 )
	{
	    uiMSG().error( "Maximum points must be positive" );
	    return false;
	}
	par.set( wmGridder2D::sKeyMaxPoints(), npoints );
    }

    return true;
}

void uiIDW::usePar( const IOPar& par )
{
    float radius;
    if (par.get(wmGridder2D::sKeySearchRadius(), radius)) {
        searchradiusfld_->setValue(radius);
        searchradiusfld_->setChecked(true);
    } else {
        searchradiusfld_->setChecked(false);
	searchradiusfld_->setValue(8000);
    }

    int npoints;
    if (par.get(wmGridder2D::sKeyMaxPoints(), npoints)) {
	maxpointsfld_->setValue(npoints);
	maxpointsfld_->setChecked(true);
    } else {
	maxpointsfld_->setChecked(false);
	maxpointsfld_->setValue(50);
    }
}

uiLTPS::uiLTPS(uiParent* p)
    : ui2D3DInterpol(p)
{
    uiString titletext( tr("Search radius %1").arg(SI().getUiXYUnitString()) );
    searchradiusfld_ = new uiGenInput( this, titletext, FloatInpSpec(8000.0) );

    maxpointsfld_ = new uiGenInput( this, tr("Maximum points per sector"), IntInpSpec(4) );
    maxpointsfld_->attach(alignedBelow, searchradiusfld_);
}

bool uiLTPS::fillPar(IOPar& par) const
{
    const float radius = searchradiusfld_->getFValue(0);
    if ( radius<=0 )
    {
	uiMSG().error( "Search radius must be positive" );
	return false;
    }
    par.set( wmGridder2D::sKeySearchRadius(), radius );

    const int npoints = maxpointsfld_->getIntValue(0);
    if ( npoints<=0 )
    {
	uiMSG().error( "Maximum points must be positive" );
	return false;
    }
    par.set( wmGridder2D::sKeyMaxPoints(), npoints );

    return true;
}

void uiLTPS::usePar(const IOPar& par)
{
    float radius = 12000;
    par.get(wmGridder2D::sKeySearchRadius(), radius);
    searchradiusfld_->setValue(radius);

    int npoints = 4;
    par.get(wmGridder2D::sKeyMaxPoints(), npoints);
    maxpointsfld_->setValue(npoints);
}

uiMBA::uiMBA(uiParent* p)
    : ui2D3DInterpol(p)
{}

uiNearestNeighbour::uiNearestNeighbour(uiParent* p)
: ui2D3DInterpol(p)
{}

  
