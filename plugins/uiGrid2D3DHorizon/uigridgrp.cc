#include "uigridgrp.h"

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
#include "emhorizon3d.h"
#include "wmgridder2d.h"

ui2D3DInterpol* ui2D3DInterpol::create( const char* methodName, uiParent* p )
{
    BufferString tmp(methodName);
    if (tmp == wmGridder2D::MethodNames[0])
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
//    horfld_->setHSzPol( uiObject::SmallVar );
    
    PositionInpSpec::Setup setup;
    PositionInpSpec spec( setup );
    stepfld_ = new uiGenInput( this, tr("Inl/Crl Step"), spec );
    stepfld_->setValue( BinID(SI().inlStep(),SI().crlStep()) );
    stepfld_->attach( alignedBelow, scopefld_ );

    {
        CtxtIOObj ctio(PickSetTranslatorGroup::ioContext());
        ctio.ctxt_.toselect_.require_.set( sKey::Type(), sKey::Polygon() );
        uiIOObjSel::Setup pssu(uiStrings::sEmptyString());
        pssu.optional(true);
        polycropfld_ = new uiIOObjSel( this,ctio.ctxt_, pssu );
        polycropfld_->setLabelText( tr("Cropping Polygon") );
        polycropfld_->attach( alignedBelow, stepfld_ );
    }
    
    methodfld_ = new uiGenInput( this, tr("Algorithm"),StringListInpSpec(wmGridder2D::MethodNames) );
    methodfld_->attach( alignedBelow, polycropfld_ );
    methodfld_->valuechanged.notify( mCB(this,uiGridGrp,methodChgCB) );
    
    for ( int idx=0; wmGridder2D::MethodNames[idx]; idx++ )
    {
        ui2D3DInterpol* methodgrp = ui2D3DInterpol::create( wmGridder2D::MethodNames[idx], this );
        if ( methodgrp )
            methodgrp->attach( alignedBelow, methodfld_ );
        methodgrps_ += methodgrp;
    }
    
    setHAlignObj( methodfld_ );
    methodChgCB( 0 );
    scopeChgCB(0);
}

void uiGridGrp::scopeChgCB(CallBacker* )
{
    horfld_->display(scopefld_->getIntValue() == wmGridder2D::Horizon);
}

void uiGridGrp::methodChgCB(CallBacker* )
{
    const int selidx = methodfld_ ? methodfld_->getIntValue( 0 ) : 0;
    for ( int idx=0; idx<methodgrps_.size(); idx++ )
    {
        if ( methodgrps_[idx] )
            methodgrps_[idx]->display( idx==selidx );
    }
}

bool uiGridGrp::fillPar( IOPar& par ) const
{
    const int scope = scopefld_->getIntValue();
    par.set (wmGridder2D::sKeyScopeType(), scope );

    if ( scope == wmGridder2D::Horizon ) {
/*        Pick::Set ps;
        BufferString msg;
        const IOObj* ioobj = polyfld_->ioobj();
        if (!PickSetTranslator::retrieve( ps, ioobj, true, msg )) {
            BufferString tmp("uiGridGrp::fillPar - error reading polyline - ");
            tmp += msg;
            ErrMsg(tmp);
            return false;
        }
        par.set(wmGridder2D::sKeyPolyScopeNodeNr(), ps.size());
        for ( int idx=0; idx<ps.size(); idx++ )
        {
            const Pick::Location& pl = ps[idx];
            const Coord bid = SI().binID2Coord().transformBackNoSnap( pl.pos_ );
            par.set( IOPar::compKey(wmGridder2D::sKeyPolyScopeNode(),idx), bid );
        }
*/    }
        
    const BinID step = stepfld_->getBinID();
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
    fltselfld_ = new uiFaultParSel( this, false );

    uiString titletext( tr("Search radius %1").arg(SI().getUiXYUnitString()) );
    radiusfld_ = new uiGenInput( this, titletext, FloatInpSpec() );
    radiusfld_->setWithCheck( true );
    radiusfld_->setChecked( false );
    radiusfld_->attach( alignedBelow, fltselfld_ );
        
    setHAlignObj( fltselfld_ );
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
    
    const TypeSet<MultiID>& selfaultids = fltselfld_->selFaultIDs();
    par.set( wmGridder2D::sKeyFaultNr(), selfaultids.size() );
    for ( int idx=0; idx<selfaultids.size(); idx++ )
        par.set( IOPar::compKey(wmGridder2D::sKeyFaultID(),idx), selfaultids[idx] );
        
    return true;
}
