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
    if (tmp == wmGridder2D::MethodNames[wmGridder2D::IDW])
        return (ui2D3DInterpol*) new uiIDW(p);
    else if (tmp == wmGridder2D::MethodNames[wmGridder2D::MBA])
	return (ui2D3DInterpol*) new uiMBA(p);
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
    
    methodfld_ = new uiGenInput( this, tr("Algorithm"),StringListInpSpec(wmGridder2D::MethodNames) );
    methodfld_->attach( alignedBelow, gridfld_ );
    methodfld_->valuechanged.notify( mCB(this,uiGridGrp,methodChgCB) );
    methodfld_->setValue(wmGridder2D::IDW);
    
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
    } else if (scopefld_->getIntValue() == wmGridder2D::ConvexHull) {
	horfld_->display(false);
	gridfld_->displayFields(false, true);
	gridfld_->setSensitive(false, true);
    } else if (scopefld_->getIntValue() == wmGridder2D::Horizon) {
        horfld_->display(true);
        gridfld_->setSensitive(false, false);
        gridfld_->displayFields(true, true);
        horChgCB(0);
    } else {
        horfld_->display(false);
        gridfld_->setSensitive(true, true);
        gridfld_->displayFields(true, true);
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
    par.set( wmGridder2D::sKeyRowStep(), step.inl() );
    par.set( wmGridder2D::sKeyColStep(), step.crl() );
    
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
    
    TrcKeySampling tks;
    par.get(wmGridder2D::sKeyRowStep(), tks.step_.first);
    par.get(wmGridder2D::sKeyColStep(), tks.step_.second);
    gridfld_->setTrcKeySampling( tks );
    
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
/*
uiCCTS::uiCCTS( uiParent* p )
: ui2D3DInterpol(p)
{
    //    fltselfld_ = new uiFaultParSel( this, false );
    
    uiString titletext( tr("Search radius %1").arg(SI().getUiXYUnitString()) );
    radiusfld_ = new uiGenInput( this, titletext, FloatInpSpec(5000.0) );
    radiusfld_->setWithCheck( false );
    
    tensionfld_ = new uiGenInput( this, tr("Tension"), FloatInpSpec(0.25) );
    tensionfld_->setWithCheck( false );
    tensionfld_->attach( alignedBelow, radiusfld_ );
    
    maxvalsfld_ = new uiGenInput(this, tr("Maximum Points"), IntInpSpec(20));
    maxvalsfld_->setWithCheck(false);
    maxvalsfld_->attach(alignedBelow, tensionfld_);
}

bool uiCCTS::fillPar( IOPar& par ) const
{
    const float radius = radiusfld_->getFValue(0);
    if ( radius<=0 )
    {
        uiMSG().error( "Search radius must be positive" );
        return false;
    }
    par.set( wmGridder2D::sKeySearchRadius(), radius );
    
    const float tension = tensionfld_->getFValue(0);
    if ( tension<=0.0 || tension>=1.0 )
    {
        uiMSG().error( "Tension must be between 0 and 1" );
        return false;
    }
    par.set( wmGridder2D::sKeyTension(), tension );
    
    const int maxvals = maxvalsfld_->getIntValue(0);
    if ( maxvals<=0 )
    {
	uiMSG().error( tr("Maximum Points must be greater than 0") );
	return false;
    }
    
    //    const TypeSet<MultiID>& selfaultids = fltselfld_->selFaultIDs();
    //    par.set( wmGridder2D::sKeyFaultNr(), selfaultids.size() );
    //    for ( int idx=0; idx<selfaultids.size(); idx++ )
    //        par.set( IOPar::compKey(wmGridder2D::sKeyFaultID(),idx), selfaultids[idx] );
    
    return true;
}

void uiCCTS::usePar( const IOPar& par )
{
    float radius;
    if (par.get(wmGridder2D::sKeySearchRadius(), radius))
        radiusfld_->setValue(radius);
    
    float tension;
    if (par.get(wmGridder2D::sKeyTension(), tension))
        tensionfld_->setValue(tension);
}

uiIter::uiIter( uiParent* p )
    : ui2D3DInterpol(p)
{ }

bool uiIter::fillPar( IOPar& par ) const
{
    return true;
}
*/
/*
uiRBF::uiRBF(uiParent* p)
    : ui2D3DInterpol(p)
{
    uiString titletext( tr("Block size %1").arg(SI().getUiXYUnitString()) );
    blocksizefld_ = new uiGenInput( this, titletext, FloatInpSpec(8000.0) );
    blocksizefld_->setWithCheck( false );
    
    percoverlapfld_ = new uiGenInput(this, tr("Percentage overlap"), IntInpSpec(20));
    percoverlapfld_->setWithCheck(false);
    percoverlapfld_->attach(alignedBelow, blocksizefld_);
    
    tensionfld_ = new uiGenInput( this, tr("Tension"), FloatInpSpec(0.25) );
    tensionfld_->setWithCheck( false );
    tensionfld_->attach( alignedBelow, blocksizefld_ );
}

bool uiRBF::fillPar(IOPar& par) const
{
    const float blocksize = blocksizefld_->getFValue(0);
    if ( blocksize<=0 )
    {
	uiMSG().error( "Block size must be positive" );
	return false;
    }
    par.set( wmGridder2D::sKeySearchRadius(), blocksize );
    
    const int percoverlap = percoverlapfld_->getIntValue(0);
    if ( percoverlap<=0 || percoverlap>=100 )
    {
	uiMSG().error( tr("Percent overlap must be between 0 and 100") );
	return false;
    }
    par.set( wmGridder2D::sKeyMaxPoints(), percoverlap );
    
    const float tension = tensionfld_->getFValue(0);
    if ( tension<=0.0 || tension>=1.0 )
    {
	uiMSG().error( "Tension must be between 0 and 1" );
	return false;
    }
    par.set( wmGridder2D::sKeyTension(), tension );
    
    return true;    
}

void uiRBF::usePar(const IOPar& par)
{
    float blocksize;
    if (par.get(wmGridder2D::sKeySearchRadius(), blocksize))
	blocksizefld_->setValue(blocksize);
    
    float percoverlap;
    if (par.get(wmGridder2D::sKeyMaxPoints(), percoverlap))
	percoverlapfld_->setValue(percoverlap);
    
    float tension;
    if (par.get(wmGridder2D::sKeyTension(), tension))
	tensionfld_->setValue(tension);
}
*/
uiMBA::uiMBA(uiParent* p)
    : ui2D3DInterpol(p)
{}


  
