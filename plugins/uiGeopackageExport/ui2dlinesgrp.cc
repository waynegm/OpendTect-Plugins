#include "ui2dlinesgrp.h"

#include "uibutton.h"
#include "uilistbox.h"
#include "uiseislinesel.h"
#include "survgeom2d.h"
#include "seisioobjinfo.h"
//#include "uiioobjselgrp.h"
//#include "ctxtioobj.h"

uiSeis2DLineSelGrp::uiSeis2DLineSelGrp( uiParent* p, OD::ChoiceMode cm, const BufferStringSet& lnnms,const TypeSet<Pos::GeomID>& geomids)
: uiSeis2DLineChoose( p, cm, lnnms, geomids )
{
    listfld_->setName("Line(s)");
}



ui2DLinesGrp::ui2DLinesGrp( uiParent* p )
: uiDlgGroup(p, tr("2D Lines"))
{
    exp2DStationsfld_ = new uiCheckBox( this, tr("Export 2D Stations"));
    exp2DStationsfld_->setChecked(false);

    BufferStringSet lnms,lnms_;
    TypeSet<Pos::GeomID> geomids, geomids_;
    SeisIOObjInfo::getLinesWithData( lnms, geomids );
    const int* idxs = lnms.getSortIndexes( false );
    if ( !idxs ) {
        lnms_ = lnms;
        geomids_ = geomids;
    }
    else {
        const int sz = lnms.size();
        for ( int idx=0; idx<sz; idx++ ) {
            lnms_.add( lnms[ idxs[idx] ]->buf() );
            geomids_.add( geomids[ idxs[idx] ] );
        }
    }
    
    lineselfld_ = new uiSeis2DLineSelGrp( this, OD::ChooseAtLeastOne, lnms, geomids );
    lineselfld_->attach( alignedBelow, exp2DStationsfld_ );
    
    setSpacing(10);
}

void ui2DLinesGrp::getGeoMids( TypeSet<Pos::GeomID>& geomids )
{
    lineselfld_->getChosen( geomids );
}

bool ui2DLinesGrp::doLineExport()
{
    TypeSet<Pos::GeomID> geomids;
    lineselfld_->getChosen( geomids );
    return (geomids.size() > 0);
}

bool ui2DLinesGrp::doStationExport()
{
    return doLineExport() && exp2DStationsfld_->isChecked();
}

void ui2DLinesGrp::reset()
{
    exp2DStationsfld_->setChecked(false);
    lineselfld_->chooseAll(false);
}
