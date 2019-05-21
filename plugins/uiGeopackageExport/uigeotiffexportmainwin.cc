#include "uigeotiffexportmainwin.h"

#include "string.h"
#include "file.h"
#include "filepath.h"
#include "oddirs.h"
#include "strmprov.h"
#include "timer.h"

#include "survinfo.h"
#include "uicoordsystem.h"
#include "uifileinput.h"
#include "uiprogressbar.h"
#include "uibutton.h"
#include "uimsg.h"
#include "ctxtioobj.h"
#include "emsurfacetr.h"
#include "iodir.h"
#include "iodirentry.h"
#include "bufstringset.h"
#include "uiioobjsel.h"
#include "uiiosurface.h"
#include "uitaskrunner.h"

#include "uigeotiffwriter.h"

uiGeotiffExportMainWin::uiGeotiffExportMainWin( uiParent* p )
    : uiDialog(p,uiDialog::Setup(getCaptionStr(),mNoDlgTitle,HelpKey("wgm","geotiff")).modal(false) )
    , hor3Dfld_(0)
{
    setCtrlStyle( OkAndCancel );
    setOkText( uiStrings::sExport() );
    setShrinkAllowed(true);

    CtxtIOObj ctio3D(EMHorizon3DTranslatorGroup::ioContext());
    const IODir iodir3D( ctio3D.ctxt_.getSelKey() );
    const IODirEntryList entries3D( iodir3D, ctio3D.ctxt_ );
    if (entries3D.size()>0) {
        expZvalue_ = new uiCheckBox(this, "Export Z value");
        expZvalue_->setChecked(true);
        
        hor3Dfld_ = new uiSurfaceRead( this, uiSurfaceRead::Setup(EMHorizon3DTranslatorGroup::sGroupName())
        .withsubsel(false).withsectionfld(false) );
        hor3Dfld_->attach(alignedBelow, expZvalue_);
        
        BufferString defseldir = FilePath(GetDataDir()).add("Misc").fullPath();
        filefld_ = new uiFileInput( this, "Output file",
                                    uiFileInput::Setup(uiFileDialog::Gen)
                                    .forread(false).filter("*.tif").defseldir(defseldir).allowallextensions(false) );
        filefld_->setDefaultExtension( "tif" );
        filefld_->attach( stretchedBelow, hor3Dfld_ );
    }
}

uiGeotiffExportMainWin::~uiGeotiffExportMainWin()
{
    
}

bool uiGeotiffExportMainWin::acceptOK( CallBacker*)
{
    if (!strlen(filefld_->fileName())) {
        uiMSG().error( tr("Please specify an output file") );
        return false;
    }
    
    MultiID hor3Did;
    hor3Did.setUdf();
    if (hor3Dfld_) {
        const IOObj* horObj = hor3Dfld_->selIOObj();
        if (horObj!=nullptr)
            hor3Did = horObj->key();
    } else
        return false;
    
    
    BufferStringSet attribs;
    if (hor3Dfld_->haveAttrSel())
        hor3Dfld_->getSelAttributes(attribs);

    uiTaskRunner taskrunner(this);
    uiGeotiffWriter gtw( filefld_->fileName() );
    
    return gtw.writeHorizon( taskrunner, hor3Did, expZvalue_->isChecked(), attribs );
}

uiString uiGeotiffExportMainWin::getCaptionStr() const
{
    return tr("Geotiff Export");
}
