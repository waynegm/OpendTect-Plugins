#include "uisurveygrp.h"

#include "uigeninput.h"
#include "uilayout.h"


uiSurveyGrp::uiSurveyGrp( uiParent* p )
: uiDlgGroup(p, tr("Survey"))
{
    expSurveyfld_ = new uiGenInput( this, tr("Export Survey Box"), BoolInpSpec(true));
}

bool uiSurveyGrp::doExport()
{
    return expSurveyfld_->getBoolValue();
}
