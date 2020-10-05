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
    wellsfld_ = new uiIOObjSelGrp( this, ctio_, uiStrings::sWells(), stup );
    wellsfld_->getManipGroup()->display(false);
    
    expWellTracks_ = new uiCheckBox(this, tr("Export Well Tracks"));
    expWellTracks_->setChecked(false);
    expWellTracks_->attach( alignedBelow, wellsfld_ );

    expMarkers_ = new uiCheckBox(this, tr("Export Markers"));
    expMarkers_->setChecked(false);
    expMarkers_->attach( alignedBelow, expWellTracks_ );
    
    inFeet_ = new uiCheckBox(this, tr("Depths in Feet"));
    inFeet_->setChecked(false);
    inFeet_->attach( rightTo, expMarkers_ );
    
}

bool uiWellsGrp::doWellExport()
{
    return (wellsfld_->nrChosen() > 0);
}

bool uiWellsGrp::doWellTrackExport()
{
    return (doWellExport() && expWellTracks_->isChecked());
}

bool uiWellsGrp::doMarkerExport()
{
    return (doWellExport() && expMarkers_->isChecked());
}

bool uiWellsGrp::doInFeet()
{
    return (doWellExport() && inFeet_->isChecked());
}

void uiWellsGrp::reset()
{
    wellsfld_->chooseAll(false);
}

void uiWellsGrp::getWellIds( TypeSet<MultiID>& wellids )
{
    wellsfld_->getChosen( wellids );
}
