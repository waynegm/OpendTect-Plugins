#ifndef uigeopackageexportmainwin_h
#define uigeopackageexportmainwin_h

#include "uidialog.h"

class uiFileInput;
class uiCheckBox;
class uiTabStack;
class uiSurveyGrp;
class ui2DLinesGrp;

mClass(uiGeopackageExport) uiGeopackageExportMainWin : public uiDialog
{ mODTextTranslationClass(uiGeopackageExportMainWin);
public:
    uiGeopackageExportMainWin(uiParent*);
    ~uiGeopackageExportMainWin();

    bool            checkCRSdefined();
protected:

    uiFileInput*    filefld_;
    uiTabStack*     tabstack_;
    uiCheckBox*     modefld_;
    uiSurveyGrp*    surveygrp_;
    ui2DLinesGrp*   linesgrp_;

    void            tabSel(CallBacker*);
    

    
/*
    CtxtIOObj&		ctio_;
    uiGMTBaseMapGrp*	basemapgrp_;
    uiGroup*		flowgrp_;
    uiListBox*		flowfld_;
    uiToolButton*	upbut_;
    uiToolButton*	downbut_;
    uiToolButton*	rmbut_;

    uiFileInput*	filefld_;
    uiPushButton*	createbut_;
    uiPushButton*	viewbut_;

    uiPushButton*	addbut_;
    uiPushButton*	editbut_;
    uiPushButton*	resetbut_;

    uiTabStack*		tabstack_;
    ObjectSet<uiGMTOverlayGrp> overlaygrps_;

    ObjectSet<GMTPar>	pars_;
    Timer*		tim_;
    bool		needsave_;
    uiBatchJobDispatcherSel*	batchfld_;

    void		createPush(CallBacker*);
    void		viewPush(CallBacker*);
    void		butPush(CallBacker*);
    void		setButStates(CallBacker*);
    void		selChg(CallBacker*);
    void		tabSel(CallBacker*);
    void		addCB(CallBacker*);
    void		editCB(CallBacker*);
    void		resetCB(CallBacker*);
    void		checkFileCB(CallBacker*);
    void		newFlow(CallBacker*);
    void		openFlow(CallBacker*);
    void		saveFlow(CallBacker*);

    bool		fillPar();
    bool		usePar( const IOPar&);
*/
    bool        acceptOK(CallBacker*);

private:

    uiString    getCaptionStr() const;
};

#endif
