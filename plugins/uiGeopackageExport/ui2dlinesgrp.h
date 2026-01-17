#pragma once

#include "uidlggroup.h"

class uiCheckBox;
namespace WMLib {
    class uiSeis2DLineSelGrp;
};

class ui2DLinesGrp : public uiDlgGroup
{ mODTextTranslationClass(ui2DLinesGrp);
public:
    ui2DLinesGrp(uiParent*);

    void getGeoMids( TypeSet<Pos::GeomID>& geomids );
    bool doLineExport();
    bool doStationExport();
    void reset();

protected:
    uiCheckBox*                 exp2DStationsfld_;
    WMLib::uiSeis2DLineSelGrp*  lineselfld_;
};
