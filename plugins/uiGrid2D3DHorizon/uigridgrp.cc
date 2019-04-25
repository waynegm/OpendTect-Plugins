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
    if (tmp == wmGridder2D::MethodNames[wmGridder2D::LocalCCTS])
        return (ui2D3DInterpol*) new uiCCTS(p);
    else if (tmp == wmGridder2D::MethodNames[wmGridder2D::IDW])
        return (ui2D3DInterpol*) new uiIDW(p);
    else {
        ErrMsg("ui2D3DInterpol::create - unrecognised method name");
        return nullptr;
    }
}

uiGridGrp::uiGridGrp( uiParent* p )
: uiDlgGroup(p, tr("Gridding Parameters"))
{
    scopefld_ = new uiGenInput(this, tr("Grid Extent"), StringListInpSpec(wmGridder2D::ScopeNames));
    scopefld_->setValue( wmGridder2D::BoundingBox );
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
    
    polycropfld_ = new WMLib::uiPolygonParSel(this, tr("Cropping Polygon"), false);
    polycropfld_->attach( alignedBelow, gridfld_ );
    
    faultpolyfld_ = new WMLib::uiPolygonParSel(this, tr("Fault Polygons"), true);
    faultpolyfld_->attach( alignedBelow, polycropfld_ );

    methodfld_ = new uiGenInput( this, tr("Algorithm"),StringListInpSpec(wmGridder2D::MethodNames) );
    methodfld_->attach( alignedBelow, faultpolyfld_ );
    methodfld_->valuechanged.notify( mCB(this,uiGridGrp,methodChgCB) );
    
    for ( int idx=0; wmGridder2D::MethodNames[idx]; idx++ )
    {
        ui2D3DInterpol* methodgrp = ui2D3DInterpol::create( wmGridder2D::MethodNames[idx], this );
        if ( methodgrp )
            methodgrp->attach( alignedBelow, methodfld_ );
        methodgrps_ += methodgrp;
    }
    
    setHAlignObj( methodfld_ );
    update();
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
    faultpolyfld_->display(methodgrps_[selidx]->canHandleFaultPolygons());
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

ui2D3DInterpol::ui2D3DInterpol( uiParent* p )
: uiGroup(p)
{}

uiIDW::uiIDW( uiParent* p )
    : ui2D3DInterpol(p)
{
//    fltselfld_ = new uiFaultParSel( this, false );

    uiString titletext( tr("Search radius %1").arg(SI().getUiXYUnitString()) );
    radiusfld_ = new uiGenInput( this, titletext, FloatInpSpec(5000.0) );
    radiusfld_->setWithCheck( true );
    radiusfld_->setChecked( true );
}

bool uiIDW::fillPar( IOPar& par ) const
{
    const float radius = radiusfld_->isChecked() ? radiusfld_->getFValue(0) : mUdf(float);
    if ( radius<=0 )
    {
        uiMSG().error( "Search radius must be positive" );
        return false;
    }
    par.set( wmIDWGridder2D::sKeySearchRadius(), radius );

    
//    const TypeSet<MultiID>& selfaultids = fltselfld_->selFaultIDs();
//    par.set( wmGridder2D::sKeyFaultNr(), selfaultids.size() );
//    for ( int idx=0; idx<selfaultids.size(); idx++ )
//        par.set( IOPar::compKey(wmGridder2D::sKeyFaultID(),idx), selfaultids[idx] );
        
    return true;
}

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
    
    //    const TypeSet<MultiID>& selfaultids = fltselfld_->selFaultIDs();
    //    par.set( wmGridder2D::sKeyFaultNr(), selfaultids.size() );
    //    for ( int idx=0; idx<selfaultids.size(); idx++ )
    //        par.set( IOPar::compKey(wmGridder2D::sKeyFaultID(),idx), selfaultids[idx] );
    
    return true;
}

/*
uiIter::uiIter( uiParent* p )
    : ui2D3DInterpol(p)
{ }

bool uiIter::fillPar( IOPar& par ) const
{
    return true;
}
*/

        
    
