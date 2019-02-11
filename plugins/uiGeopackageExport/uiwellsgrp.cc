#include "uiwellsgrp.h"

#include "uibutton.h"
#include "ctxtioobj.h"
#include "uiioobjselgrp.h"
#include "uiioobjmanip.h"
#include "welltransl.h"
#include "bufstringset.h"

uiWellsGrp::uiWellsGrp( uiParent* p )
: uiDlgGroup(p, tr("Wells")), ctio_(WellTranslatorGroup::ioContext())
{
    uiIOObjSelGrp::Setup stup; 
    stup.choicemode_ = OD::ChooseZeroOrMore;
    wellsfld_ = new uiIOObjSelGrp( this, ctio_, "Well(s)", stup );
    wellsfld_->getManipGroup()->display(false);
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
