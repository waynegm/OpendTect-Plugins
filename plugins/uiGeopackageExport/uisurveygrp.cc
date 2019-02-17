#include "uisurveygrp.h"

#include "uigeninput.h"
#include "uilayout.h"


uiSurveyGrp::uiSurveyGrp( uiParent* p )
: uiDlgGroup(p, tr("Survey"))
{
    expsurvey_ = true;
    expSurveyfld_ = new uiGenInput( this, tr("Export Survey Box"), BoolInpSpec(expsurvey_));
}

bool uiSurveyGrp::doExport()
{
    return expsurvey_;
}
