#include "uisurveygrp.h"

#include "uibutton.h"

uiSurveyGrp::uiSurveyGrp( uiParent* p )
: uiDlgGroup(p, tr("Survey"))
{
    expSurveyfld_ = new uiCheckBox( this, tr("Export Survey Box"));
    expSurveyfld_->setChecked(true);
}

bool uiSurveyGrp::doExport()
{
    return expSurveyfld_->isChecked();
}
