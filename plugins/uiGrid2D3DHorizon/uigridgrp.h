#ifndef uigridgrp_h
#define uigridgrp_h

#include "uidlggroup.h"

class uiGenInput;
class uiPolygonParSel;
class uiSurfaceRead;
class ui2D3DInterpol;
class uiFaultParSel;
class IOPar;
class BufferStringSet;
namespace WMLib {
    class uiPolygonParSel;
    class ui3DRangeGrp;
};

class uiGridGrp : public uiDlgGroup
{ mODTextTranslationClass(uiGridGrp);
public:
    uiGridGrp(uiParent*);
    ~uiGridGrp();
    
    bool                fillPar( IOPar& par ) const;
    void                usePar( const IOPar& par );
    void                update();
    
protected:
    uiGenInput*                 scopefld_;
    uiSurfaceRead*              horfld_;
    WMLib::uiPolygonParSel*     polycropfld_;
    WMLib::ui3DRangeGrp*        gridfld_;
    uiGenInput*                 methodfld_;
    WMLib::uiPolygonParSel*     faultpolyfld_;
    ObjectSet<ui2D3DInterpol>   methodgrps_;
    
    void                scopeChgCB(CallBacker*);
    void                horChgCB(CallBacker*);
    void                methodChgCB(CallBacker*);
};

class ui2D3DInterpol : public uiGroup
{mODTextTranslationClass(ui2D3DInterpol);
public:
    static ui2D3DInterpol*  create( const char* methodName, uiParent* p );
    
    virtual             ~ui2D3DInterpol()	{}
    
    virtual bool        fillPar(IOPar&) const	{ return false; }
    virtual void        usePar(const IOPar&)	{ }
    
    virtual bool        canHandleFaultSurfaces() const { return false; }
    virtual bool        canHandleFaultPolygons() const { return false; }
    
protected:
    ui2D3DInterpol(uiParent*);
};

class uiIDW : public ui2D3DInterpol
{ mODTextTranslationClass(uiIDW);
public:
    uiIDW(uiParent*);
    
    virtual bool    fillPar(IOPar&) const;
    virtual void    usePar(const IOPar&);
    
protected:
    uiGenInput*         blocksizefld_;
};

class uiRBF : public ui2D3DInterpol
{ mODTextTranslationClass(uiRBF);
public:
    uiRBF(uiParent*);
    
    virtual bool    fillPar(IOPar&) const;
    virtual void    usePar(const IOPar&);

protected:
    uiGenInput*         blocksizefld_;
    uiGenInput*		percoverlapfld_;
    uiGenInput*         tensionfld_;
};    

class uiMBA : public ui2D3DInterpol
{ mODTextTranslationClass(uiMBA);
public:
    uiMBA(uiParent*);
    
protected:
};

#endif
