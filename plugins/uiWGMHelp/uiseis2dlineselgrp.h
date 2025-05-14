#pragma once

#include "uiwgmhelpmod.h"
#include "bufstringset.h"
#include "odcommonenums.h"
#include "posinfo2dsurv.h"
#include "uigroup.h"
#include "callback.h"

class uiListBox;
class uiListBoxFilter;
class uiListBoxChoiceIO;

namespace WMLib
{

mExpClass(uiWGMHelp) uiSeis2DLineSelGrp: public uiGroup
{
public:
    uiSeis2DLineSelGrp(uiParent*, OD::ChoiceMode);
    uiSeis2DLineSelGrp(uiParent*, OD::ChoiceMode, const BufferStringSet&, const TypeSet<Pos::GeomID>&);

    ~uiSeis2DLineSelGrp();

    void    setInput(const BufferStringSet& lnms, const TypeSet<Pos::GeomID>& geomid);
    int     nrChosen() const;
    void    getChosen(TypeSet<Pos::GeomID>&) const;
    void    getChosen(BufferStringSet&) const;
    void    setChosen(const TypeSet<Pos::GeomID>&);
    void    setChosen(const BufferStringSet&);
    void    chooseAll(bool yn=true);

    void    clear();

    Notifier<uiSeis2DLineSelGrp> selectionDone;

protected:
    BufferStringSet lnms_;
    TypeSet<Pos::GeomID> geomids_;

    uiListBox*              listfld_;
    uiListBoxFilter*        filtfld_;
    uiListBoxChoiceIO*      lbchoiceio_ = nullptr;

    void                    init(OD::ChoiceMode);
    void                    readChoiceDone(CallBacker*);
    void                    writeChoiceReq(CallBacker*);
    void                    selChgCB(CallBacker*);

};
}; //namespace
