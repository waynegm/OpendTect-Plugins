#include "uipolylinesgrp.h"

#include "uibutton.h"
#include "ctxtioobj.h"
#include "uiioobjselgrp.h"
#include "uiioobjmanip.h"
#include "picksettr.h"
#include "bufstringset.h"

uiPolyLinesGrp::uiPolyLinesGrp( uiParent* p )
: uiDlgGroup(p, tr("Polylines")), ctio_(PickSetTranslatorGroup::ioContext())
{
    ctio_.ctxt_.toselect_.require_.set( sKey::Type(), sKey::Polygon() );
    uiIOObjSelGrp::Setup stup; 
    stup.choicemode_ = OD::ChooseZeroOrMore;
    linesfld_ = new uiIOObjSelGrp( this, ctio_, uiStrings::sPolyLine(mPlural), stup );
    linesfld_->getManipGroup()->display(false);
}

bool uiPolyLinesGrp::doLineExport()
{
    return (linesfld_->nrChosen() > 0);
}

void uiPolyLinesGrp::reset()
{
    linesfld_->chooseAll(false);
}

void uiPolyLinesGrp::getLineIds( TypeSet<MultiID>& lineids )
{
    linesfld_->getChosen( lineids );
}
