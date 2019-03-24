#ifndef uigridgrp_h
#define uigridgrp_h

#include "factory.h"
#include "uidlggroup.h"

class uiGenInput;
class uiIOObjSel;
class ui2D3DInterpol;
class uiFaultParSel;
class IOPar;

class uiGridGrp : public uiDlgGroup
{ mODTextTranslationClass(uiGridGrp);
public:
    uiGridGrp(uiParent*);
    ~uiGridGrp() {}
    
    bool                fillPar( IOPar& par ) const;
    
protected:
    uiGenInput*         scopefld_;
    uiIOObjSel*         polyfld_;
    uiGenInput*         stepfld_;
    uiGenInput*         methodfld_;
    ObjectSet<ui2D3DInterpol>  methodgrps_;
    
    void                scopeChgCB(CallBacker*);
    void                methodChgCB(CallBacker*);
};

class ui2D3DInterpol : public uiGroup
{mODTextTranslationClass(ui2D3DInterpol);
public:
    mDefineFactory1ParamInClass(ui2D3DInterpol,uiParent*,factory);
    
    virtual			~ui2D3DInterpol()	{}
    
    virtual bool		fillPar(IOPar&) const	{ return false; }
    virtual bool		usePar(const IOPar&)	{ return false; }
    
    virtual bool		canHandleFaults() const { return false; }
    
protected:
    ui2D3DInterpol(uiParent*);
};

class uiIDW : public ui2D3DInterpol
{ mODTextTranslationClass(uiIDW);
public:
    uiIDW(uiParent*);
    
    static void initClass();
    static      ui2D3DInterpol*	create(uiParent*);
    
    virtual bool    fillPar(IOPar&) const;
    virtual bool    usePar(const IOPar&);
    
    virtual bool    canHandleFaults() const { return true; }
    const char*     factoryKeyword() const;
protected:
    uiGenInput*     radiusfld_;
    uiGenInput*     powfld_;
    uiFaultParSel*  fltselfld_;
};


#endif
