#include "uirandomlinesgrp.h"

#include "uibutton.h"
#include "ctxtioobj.h"
#include "uiioobjselgrp.h"
#include "randomlinetr.h"
#include "bufstringset.h"

uiRandomLinesGrp::uiRandomLinesGrp( uiParent* p )
: uiDlgGroup(p, tr("Random Lines")), ctio_(RandomLineSetTranslatorGroup::ioContext())
{
    uiIOObjSelGrp::Setup stup; 
    stup.choicemode_ = OD::ChooseAtLeastOne;
    stup.allowreloc_ = false;
    stup.allowremove_ = false;
    stup.withinserters_ = false;
    stup.withwriteopts_ = false;
    stup.confirmoverwrite_ = false;
    linesfld_ = new uiIOObjSelGrp( this, ctio_, "Line(s)", stup );
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
