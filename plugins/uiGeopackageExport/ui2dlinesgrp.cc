#include "ui2dlinesgrp.h"

#include "uiseis2dlineselgrp.h"

#include "bufstringset.h"
#include "uibutton.h"
#include "uilistbox.h"
#include "survgeom2d.h"
#include "seisioobjinfo.h"

ui2DLinesGrp::ui2DLinesGrp( uiParent* p )
: uiDlgGroup(p, tr("2D Lines"))
{
    exp2DStationsfld_ = new uiCheckBox( this, tr("Export 2D Stations"));
    exp2DStationsfld_->setChecked(false);

    lineselfld_ = new WMLib::uiSeis2DLineSelGrp( this, OD::ChooseZeroOrMore );
    lineselfld_->attach( alignedBelow, exp2DStationsfld_ );
}

void ui2DLinesGrp::getGeoMids( TypeSet<Pos::GeomID>& geomids )
{
    lineselfld_->getChosen( geomids );
}

bool ui2DLinesGrp::doLineExport()
{
    return (lineselfld_->nrChosen() > 0);
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
