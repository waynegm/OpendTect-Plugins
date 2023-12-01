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
#include "seisioobjinfo.h"
#include "uiioobjsel.h"
#include "uiioobjselgrp.h"
#include "uiiosurface.h"
#include "uiseissel.h"
#include "uitaskrunner.h"

#include "uigeotiffwriter.h"

uiGeotiffExportMainWin::uiGeotiffExportMainWin( uiParent* p )
    : uiDialog(p,uiDialog::Setup(getCaptionStr(),mNoDlgTitle,HelpKey("wgm","geotiff")).modal(false) )
{
    setCtrlStyle( OkAndCancel );
    setOkText( uiStrings::sExport() );
    setShrinkAllowed(true);

    expTypefld_ = new uiGenInput( this, tr("Export"), BoolInpSpec(true,uiStrings::sHorizon(), uiStrings::sZSlice()));

    expZvalue_ = new uiCheckBox(this, tr("Export Z value"));
    expZvalue_->setChecked(true);
    expZvalue_->attach(alignedBelow, expTypefld_);

    hor3Dfld_ = new uiSurfaceRead( this, uiSurfaceRead::Setup(EMHorizon3DTranslatorGroup::sGroupName())
        .withsubsel(false).withsectionfld(false) );
    hor3Dfld_->attach(alignedBelow, expZvalue_);

    IOObjContext ctxt( uiSeisSel::ioContext( Seis::Vol, true ) );
    uiIOObjSelGrp::Setup sgsu( OD::ChooseZeroOrMore );
    sgsu.allowremove( false );
    ctxt.forread_ = true;
    inpfld_ = new uiIOObjSelGrp( this, ctxt, tr("Input Cubes"), sgsu );
    inpfld_->attach(alignedBelow, expTypefld_);

    uiString title ( uiStrings::phrJoinStrings(uiStrings::sZValue(), SI().getUiZUnitString()) );
    auto zrg = SI().zRange(true);
    zrg.scale(SI().showZ2UserFactor());
    expZfld_ = new uiGenInput(this, title, FloatInpSpec(zrg.center(), zrg));
    expZfld_->attach(alignedBelow, inpfld_);

    BufferString defseldir = FilePath(GetDataDir()).add("Misc").fullPath();
    filefld_ = new uiFileInput( this, uiStrings::sOutputFile(),
				uiFileInput::Setup(uiFileDialog::Gen)
				.forread(false).filter("*.tif").defseldir(defseldir).allowallextensions(false) );
    filefld_->setDefaultExtension( "tif" );
    filefld_->attach( stretchedAbove, expTypefld_ );

    mAttachCB( postFinalize(), uiGeotiffExportMainWin::initCB );

}

uiGeotiffExportMainWin::~uiGeotiffExportMainWin()
{
    detachAllNotifiers();
}

bool uiGeotiffExportMainWin::acceptOK( CallBacker*)
{
    if (!strlen(filefld_->fileName())) {
        uiMSG().error( tr("Please specify an output file") );
        return false;
    }

    const bool forhorizon = expTypefld_->getBoolValue();
    uiRetVal uirv;
    uiTaskRunner taskrunner(this);
    uiGeotiffWriter gtw( filefld_->fileName() );
    if ( forhorizon )
    {
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

	uirv = gtw.writeHorizon( taskrunner, hor3Did, expZvalue_->isChecked(), attribs );
    }
    else
    {
	TypeSet<MultiID> seisids;
	inpfld_->getChosen( seisids );
	const float slicetime = expZfld_->getFValue();
	uiUserShowWait uisw( this, uiStrings::sExport() );

	uirv = gtw.writeZSlices( taskrunner, seisids, slicetime );
    }

    if (!uirv.isOK())
	uiMSG().errorWithDetails( uirv, tr("Error saving geotiff"));

    return uirv.isOK();
}

uiString uiGeotiffExportMainWin::getCaptionStr() const
{
    return tr("Geotiff Export");
}

void uiGeotiffExportMainWin::initCB( CallBacker* )
{
    mAttachCB(expTypefld_->valueChanged, uiGeotiffExportMainWin::updateuiCB);
    mAttachCB(inpfld_->selectionChanged, uiGeotiffExportMainWin::inputselCB);

    updateuiCB(nullptr);
}

void uiGeotiffExportMainWin::updateuiCB( CallBacker* )
{
    const bool forhorizon = expTypefld_->getBoolValue();
    expZvalue_->display(forhorizon);
    hor3Dfld_->display(forhorizon);
    inpfld_->display(!forhorizon);
    expZfld_->display(!forhorizon);
}

void uiGeotiffExportMainWin::inputselCB( CallBacker* )
{
    TypeSet<MultiID> seisids;
    inpfld_->getChosen(seisids);
    StepInterval<float> zrg = SI().zRange(true);
    for ( const auto& seisid : seisids )
    {
	SeisIOObjInfo seisinfo(seisid);
	TrcKeyZSampling tkzs;
	seisinfo.getRanges(tkzs);
	Pos::intersectF(zrg, tkzs.zsamp_, zrg);
    }
    zrg.scale(SI().showZ2UserFactor());
    const float curzval = expZfld_->getFValue(zrg.center());
    expZfld_->newSpec(FloatInpSpec(zrg.includes(curzval, false) ? curzval : zrg.center(), zrg), 0);
}
