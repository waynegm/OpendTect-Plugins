 
#ifndef ui2dlinesgrp_h
#define ui2dlinesgrp_h

#include "uidlggroup.h"

class uiCheckBox;
class uiSeis2DLineSel;

class ui2DLinesGrp : public uiDlgGroup
{ mODTextTranslationClass(ui2DLinesGrp);
public:
    ui2DLinesGrp(uiParent*);
    
    void getGeoMids( TypeSet<Pos::GeomID>& geomids );
    bool doLineExport();
    void reset();
    
protected:
    uiCheckBox* exp2DLinesfld_;
    uiSeis2DLineSel*	lineselfld_;
    
};




#endif
