#pragma once

#include "uidlggroup.h"
#include "ctxtioobj.h"

class uiCheckBox;
class CtxtIOObj;
class uiIOObjSelGrp;

class uiWellsGrp : public uiDlgGroup
{ mODTextTranslationClass(uiWellsGrp);
public:
    uiWellsGrp(uiParent*);
    ~uiWellsGrp() {}

    bool doWellExport();
    bool doWellTrackExport();
    bool doMarkerExport();
    bool doInFeet();
    void getWellIds( TypeSet<MultiID>& wellids );
    void reset();

protected:
    CtxtIOObj       ctio_;
    uiIOObjSelGrp*  wellsfld_;
    uiCheckBox*     expWellTracks_;
    uiCheckBox*     expMarkers_;
    uiCheckBox*     inFeet_;
};
