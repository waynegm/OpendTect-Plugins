#include "ui2dlinesgrp.h"

#include "uibutton.h"
#include "uiseislinesel.h"

ui2DLinesGrp::ui2DLinesGrp( uiParent* p )
: uiDlgGroup(p, tr("2D Lines"))
{
    exp2DLinesfld_ = new uiCheckBox( this, tr("Export 2DLines"));
    exp2DLinesfld_->setChecked(true);

    lineselfld_ = new uiSeis2DLineSel( this, true );
    lineselfld_->attach( alignedBelow, exp2DLinesfld_ );
    
}

void ui2DLinesGrp::getGeoMids( TypeSet<Pos::GeomID>& geomids )
{
    lineselfld_->getSelGeomIDs( geomids );
}

bool ui2DLinesGrp::doLineExport()
{
    return exp2DLinesfld_->isChecked();
}

void ui2DLinesGrp::reset()
{
    exp2DLinesfld_->setChecked(true);
    lineselfld_->clearSelection();
}
