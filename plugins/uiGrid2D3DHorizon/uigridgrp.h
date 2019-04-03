#ifndef uigridgrp_h
#define uigridgrp_h

#include "uidlggroup.h"

class uiGenInput;
class uiIOObjSel;
class uiSurfaceRead;
class ui2D3DInterpol;
class uiFaultParSel;
class IOPar;
class BufferStringSet;

class uiGridGrp : public uiDlgGroup
{ mODTextTranslationClass(uiGridGrp);
public:
    uiGridGrp(uiParent*);
    ~uiGridGrp() {}
    
    bool                fillPar( IOPar& par ) const;
    
protected:
    uiGenInput*         scopefld_;
    uiSurfaceRead*      horfld_;
    uiIOObjSel*         polycropfld_;
    uiGenInput*         stepfld_;
    uiGenInput*         methodfld_;
    ObjectSet<ui2D3DInterpol>  methodgrps_;
    
    void                scopeChgCB(CallBacker*);
    void                methodChgCB(CallBacker*);
};

class ui2D3DInterpol : public uiGroup
{mODTextTranslationClass(ui2D3DInterpol);
public:
    static ui2D3DInterpol*  create( const char* methodName, uiParent* p );
    
    virtual             ~ui2D3DInterpol()	{}
    
    virtual bool        fillPar(IOPar&) const	{ return false; }
    virtual bool        usePar(const IOPar&)	{ return false; }
    
    virtual bool        canHandleFaults() const { return false; }
    
protected:
    ui2D3DInterpol(uiParent*);
};

class uiIDW : public ui2D3DInterpol
{ mODTextTranslationClass(uiIDW);
public:
    uiIDW(uiParent*);
    
    virtual bool    fillPar(IOPar&) const;
//    virtual bool    usePar(const IOPar&);
    
    virtual bool    canHandleFaults() const { return true; }

protected:
    uiGenInput*     radiusfld_;
    uiFaultParSel*  fltselfld_;
};


#endif
