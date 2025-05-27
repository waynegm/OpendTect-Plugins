#ifndef uibufferstringsetselgrp_h
#define uibufferstringsetselgrp_h

#include "uiwgmhelpmod.h"
#include "bufstringset.h"
#include "uigroup.h"
#include "callback.h"

class uiListBox;
class uiListBoxFilter;
class uiListBoxChoiceIO;

namespace WMLib
{
    
mExpClass(uiWGMHelp) uiBufferStringSetSelGrp: public uiGroup
{
public:
    uiBufferStringSetSelGrp(uiParent*, OD::ChoiceMode);
    uiBufferStringSetSelGrp(uiParent*, OD::ChoiceMode, const BufferStringSet&);
    
    ~uiBufferStringSetSelGrp();
    
    void    setInput(const BufferStringSet& lnms);
    int     nrChosen() const;
    void    getChosen(BufferStringSet&) const;
    void    setChosen(const BufferStringSet&);
    void    chooseAll(bool yn=true);
    
    void    clear();
    
    Notifier<uiBufferStringSetSelGrp> selectionDone;
    
protected:
    BufferStringSet lnms_;
    
    uiListBox*              listfld_;
    uiListBoxFilter*        filtfld_;
    uiListBoxChoiceIO*      lbchoiceio_ = nullptr;
    
    void                    init(OD::ChoiceMode);
    void                    selChgCB(CallBacker*);
    
};
}; //namespace

#endif
