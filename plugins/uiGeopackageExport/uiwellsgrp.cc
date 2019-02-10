#include "uiwellsgrp.h"

#include "uibutton.h"
#include "ctxtioobj.h"
#include "uiioobjselgrp.h"
#include "welltransl.h"
#include "bufstringset.h"

uiWellsGrp::uiWellsGrp( uiParent* p )
: uiDlgGroup(p, tr("Wells")), ctio_(WellTranslatorGroup::ioContext())
{
    uiIOObjSelGrp::Setup stup; 
    stup.choicemode_ = OD::ChooseAtLeastOne;
    stup.allowreloc_ = false;
    stup.allowremove_ = false;
    stup.withinserters_ = false;
    stup.withwriteopts_ = false;
    stup.confirmoverwrite_ = false;
    wellsfld_ = new uiIOObjSelGrp( this, ctio_, "Well(s)", stup );
}

bool uiWellsGrp::doWellExport()
{
    return (wellsfld_->nrChosen() > 0);
}

void uiWellsGrp::reset()
{
    wellsfld_->chooseAll(false);
}

void uiWellsGrp::getWellIds( TypeSet<MultiID>& wellids )
{
    wellsfld_->getChosen( wellids );
}
