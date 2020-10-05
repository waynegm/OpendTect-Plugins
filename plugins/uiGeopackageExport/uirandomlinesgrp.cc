#include "uirandomlinesgrp.h"

#include "uibutton.h"
#include "ctxtioobj.h"
#include "uiioobjselgrp.h"
#include "uiioobjmanip.h"
#include "randomlinetr.h"
#include "bufstringset.h"

uiRandomLinesGrp::uiRandomLinesGrp( uiParent* p )
: uiDlgGroup(p, tr("Random Lines")), ctio_(RandomLineSetTranslatorGroup::ioContext())
{
    uiIOObjSelGrp::Setup stup; 
    stup.choicemode_ = OD::ChooseZeroOrMore;
    linesfld_ = new uiIOObjSelGrp( this, ctio_, uiStrings::sLine(2), stup );
    linesfld_->getManipGroup()->display(false);
}

bool uiRandomLinesGrp::doLineExport()
{
    return (linesfld_->nrChosen() > 0);
}

void uiRandomLinesGrp::reset()
{
    linesfld_->chooseAll(false);
}

void uiRandomLinesGrp::getLineIds( TypeSet<MultiID>& lineids )
{
    linesfld_->getChosen( lineids );
}
