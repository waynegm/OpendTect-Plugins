#include "uiwmhorsavefieldgrp.h"

#include "uimsg.h"
#include "uistring.h"
#include "uigeninput.h"
#include "uiioobjsel.h"
#include "emhorizon3d.h"
#include "emmanager.h"
#include "emioobjinfo.h"
#include "ioobj.h"
#include "emsurfacegeometry.h"
#include "rowcolsurface.h"

uiwmHorSaveFieldGrp::uiwmHorSaveFieldGrp(uiParent* p)
    : uiHorSaveFieldGrp(p, 0, false, false)
{
    savefld_->display(false);
    outputfld_->setConfirmOverwrite(true);
}

bool uiwmHorSaveFieldGrp::createNewHorizon()
{
    EM::EMManager& em = EM::EMM();
    EM::IOObjInfo eminfo(outputfld_->key());
//    if (eminfo.isOK()) {
//        uiString msg = tr("Horizon: %1 already exists.\nDo you wish to overwrite?").arg(em.objectName(outputfld_->key()));
//        if (!uiMSG().askGoOn(msg,uiStrings::sOverwrite(),uiStrings::sCancel()))
//            return false;
//    }
    if ( newhorizon_ ) {
        newhorizon_->unRef();
        newhorizon_ = 0;
    }

    EM::ObjectID objid = em.createObject(EM::Horizon3D::typeStr(), outputfld_->getInput());

    mDynamicCastGet(EM::Horizon*,horizon,em.getObject(objid));
    if ( !horizon )
        return false;

    newhorizon_ = horizon;
    newhorizon_->ref();
    newhorizon_->setMultiID( outputfld_->ioobj()->key() );

    return true;    
}

bool uiwmHorSaveFieldGrp::setHorRange( const Interval<int>& newinlrg, const Interval<int>& newcrlrg )
{
/*    mDynamicCastGet(EM::Horizon3D*,hor3D,newhorizon_);
    if (!hor3D || !hor3D->geometry().nrSections())
        return false;

    EM::SectionID sid = hor3D->geometry().sectionID( 0 );
    hor3D->geometry().sectionGeometry(sid)->expandWithUdf()
    
    mDynamicCastGet( Geometry::ParametricSurface*, surf, hor->sectionGeometry( sid ) );
    if ( !surf )
        return false;
    uiMSG().message("in setHorRange");
    StepInterval<int> rowrg = hor->geometry().rowRange( sid );
    StepInterval<int> colrg = hor->geometry().colRange( sid, -1 );

    while ( colrg.start-colrg.step >= newcrlrg.start ) {
        const int newcol = colrg.start-colrg.step;
        surf->insertCol( newcol );
        colrg.start = newcol;
    }

    if ( colrg.start < newcrlrg.start )
        surf->removeCol( colrg.start, newcrlrg.start-1 );

    while ( colrg.stop+colrg.step <= newcrlrg.stop ) {
        const int newcol = colrg.stop+colrg.step;
        surf->insertCol( newcol );
        colrg.stop = newcol;
    }

    if ( colrg.stop > newcrlrg.stop )
        surf->removeCol( newcrlrg.stop+1, colrg.stop );

    while ( rowrg.start-rowrg.step >= newinlrg.start ) {
        const int newrow = rowrg.start-rowrg.step;
        surf->insertRow( newrow );
        rowrg.start = newrow;
    }

    if ( rowrg.start < newinlrg.start )
        surf->removeRow( rowrg.start, newinlrg.start-1 );

    while ( rowrg.stop+rowrg.step <= newinlrg.stop ) {
        const int newrow = rowrg.stop+rowrg.step;
        surf->insertRow( newrow );
        rowrg.stop = newrow;
    }

    if ( rowrg.stop > newinlrg.stop )
        surf->removeRow( newinlrg.stop+1, rowrg.stop );

    uiMSG().message(tr("in setHorRange\nInl: %1 - %2\nCrl: %3 - %4").arg(rowrg.start).arg(rowrg.stop)
    .arg(colrg.start).arg(colrg.stop));
*/    
    return true;
}
