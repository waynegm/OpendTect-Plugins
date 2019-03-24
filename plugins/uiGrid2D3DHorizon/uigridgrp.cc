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
#include "wmgridder2d.h"

uiGridGrp::uiGridGrp( uiParent* p )
: uiDlgGroup(p, tr("Gridding Parameters"))
{
    uiStringSet scopes;
    scopes += tr("Bounding box");
    scopes += tr("Convex hull");
    scopes += tr("Polygon");
    scopefld_ = new uiGenInput(this, tr("Scope"), StringListInpSpec(scopes));
    scopefld_->setValue( 1 );
    scopefld_->valuechanged.notify( mCB(this,uiGridGrp,scopeChgCB));
    
    {
        CtxtIOObj ctio(PickSetTranslatorGroup::ioContext());
        ctio.ctxt_.toselect_.require_.set( sKey::Type(), sKey::Polygon() );
        polyfld_ = new uiIOObjSel( this,ctio.ctxt_,uiIOObjSel::Setup(uiStrings::sEmptyString()) );
        polyfld_->setCaption( uiStrings::sEmptyString() );
        polyfld_->attach( rightOf, scopefld_ );
        polyfld_->setHSzPol( uiObject::SmallVar );
    }
    
    PositionInpSpec::Setup setup;
    PositionInpSpec spec( setup );
    stepfld_ = new uiGenInput( this, tr("Inl/Crl Step"), spec );
    stepfld_->setValue( BinID(SI().inlStep(),SI().crlStep()) );
    stepfld_->attach( alignedBelow, scopefld_ );

    const BufferStringSet& methods = ui2D3DInterpol::factory().getNames();
    methodfld_ = new uiGenInput( this, tr("Algorithm"),StringListInpSpec(ui2D3DInterpol::factory().getUserNames() ) );
    methodfld_->attach( alignedBelow, stepfld_ );
    methodfld_->valuechanged.notify( mCB(this,uiGridGrp,methodChgCB) );
    
    for ( int idx=0; idx<methods.size(); idx++ )
    {
        ui2D3DInterpol* methodgrp = ui2D3DInterpol::factory().create( methods[idx]->buf(), this, true );
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
    polyfld_->display(scopefld_->getIntValue() == 2);
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

    if ( scope == wmGridder2D::Polygon ) {
        Pick::Set ps;
        BufferString msg;
        const IOObj* ioobj = polyfld_->ioobj();
        if (!PickSetTranslator::retrieve( ps, ioobj, true, msg )) {
            BufferString tmp("uiGridGrp::fillPar - error reading polyline - ");
            tmp += msg;
            ErrMsg(tmp);
            return false;
        }
        for ( int idx=0; idx<ps.size(); idx++ )
        {
            const Pick::Location& pl = ps[idx];
            const Coord bid = SI().binID2Coord().transformBackNoSnap( pl.pos_ );
            par.set( IOPar::compKey(wmGridder2D::sKeyPolyNode(),idx), bid );
        }
    }
        
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
    const ui2D3DInterpol* methodgrp = methodgrps_[methodidx];
    par.set( wmGridder2D::sKeyMethod(), methodgrp->factoryKeyword() );
    return methodgrp->fillPar( par );
}

uiIDW::uiIDW( uiParent* p )
    : ui2D3DInterpol(p)
{
    fltselfld_ = new uiFaultParSel( this, false );
        
    uiString titletext( tr("Search radius %1").arg(SI().getUiXYUnitString()) );
    radiusfld_ = new uiGenInput( this, titletext, FloatInpSpec() );
    radiusfld_->setWithCheck( true );
    radiusfld_->setChecked( true );
    radiusfld_->attach( alignedBelow, fltselfld_ );
        
    powfld_ = new uiGenInput( this, tr("Power"), FloatInpSpec(2.0f) );
    powfld_->setWithCheck( true );
    powfld_->setChecked( true );
    powfld_->attach( alignedBelow, radiusfld_ );
    
    setHAlignObj( radiusfld_ );
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
    
    const float power = powfld_->isChecked() ? powfld_->getFValue(0) : 2.0f;
    if ( power<0 )
    {
        uiMSG().error( "Power must be positive" );
        return false;
    }
    par.set( wmIDWGridder2D::sKeyPower(), power );
    
    const TypeSet<MultiID>& selfaultids = fltselfld_->selFaultIDs();
    par.set( wmGridder2D::sKeyFaultNr(), selfaultids.size() );
    for ( int idx=0; idx<selfaultids.size(); idx++ )
        par.set( IOPar::compKey(wmGridder2D::sKeyFaultID(),idx),
                 selfaultids[idx] );
        
    return true;
}
