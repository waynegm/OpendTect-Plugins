#ifndef ui3drangegrp_h
#define ui3drangegrp_h

#include "uiwgmhelpmod.h"
#include "uicompoundparsel.h"
#include "typeset.h"
#include "multiid.h"
#include "bufstringset.h"

class uiGenInput;
class Callbacker;
class uiPushButton;

namespace WMLib
{
    
mExpClass(uiWGMHelp) uiPolygonParSel : public uiCompoundParSel
{ mODTextTranslationClass(uiPolygonParSel);
public:
    uiPolygonParSel(uiParent*, const uiString&, bool multisel=true);
    ~uiPolygonParSel();
    
    void                    setSelectedPolygons(const TypeSet<MultiID>&);
    BufferString            getSummary() const;
    const TypeSet<MultiID>&	selPolygonIDs() const { return selpolygonids_; }
    
    void                    setEmpty();
    void                    hideClearButton(bool yn=true);
    
    Notifier<uiPolygonParSel>	selChange;
    
protected:
    void                    clearPush(CallBacker*);
    void                    doDlg(CallBacker*);
    
    BufferStringSet         selpolygonnms_;
    TypeSet<MultiID>        selpolygonids_;
    bool                    multisel_;
    
    uiPushButton*           clearbut_;
};
}; //namespace

#endif
