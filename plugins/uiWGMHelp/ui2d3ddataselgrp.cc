#include "ui2d3ddataselgrp.h"

#include "survinfo.h"
#include "seisioobjinfo.h"
#include "uibutton.h"
#include "uipossubsel.h"

#include "uiseis2dlineselgrp.h"

using namespace WMLib;

ui2D3DDataSelGrp::ui2D3DDataSelGrp( uiParent* p )
    : uiDlgGroup(p, tr("Input Data"))
    , line2dselfld_(nullptr)
    , include3dfld_(nullptr)
{
    if (SI().has2D()) {
	BufferStringSet lnms;
	TypeSet<Pos::GeomID> geomids;
	SeisIOObjInfo::getLinesWithData(lnms, geomids);
	if (lnms.size() > 0 )
	    line2dselfld_ = new WMLib::uiSeis2DLineSelGrp(this, OD::ChooseZeroOrMore);
    }

    if (SI().has3D()) {
	include3dfld_ = new uiCheckBox(this, tr("Include 3D survey area"));
	if ( line2dselfld_ )
	    include3dfld_->attach(alignedBelow, line2dselfld_);

	include3dfld_->setChecked(true);
	mAttachCB(include3dfld_->activated, ui2D3DDataSelGrp::include3DCB);
	subsel3dfld_ = new uiPosSubSel( this, uiPosSubSel::Setup(false,false).withstep(false) );
	subsel3dfld_->attach( alignedBelow, include3dfld_ );
    }

    mAttachCB( postFinalise(), ui2D3DDataSelGrp::initGrp );
}

ui2D3DDataSelGrp::~ui2D3DDataSelGrp()
{
    detachAllNotifiers();
}

void ui2D3DDataSelGrp::initGrp(  CallBacker* )
{
    include3DCB(nullptr);
}

int ui2D3DDataSelGrp::num2DLinesChosen() const
{
    if ( line2dselfld_ )
	return line2dselfld_->nrChosen();
    else
	return 0;
}

void ui2D3DDataSelGrp::get2DGeomIDs( TypeSet<Pos::GeomID>& geomids ) const
{
    geomids.erase();
    if ( line2dselfld_ )
	line2dselfld_->getChosen( geomids );
}

void ui2D3DDataSelGrp::get3Dsel( TrcKeyZSampling& envelope ) const
{
    envelope.setEmpty();
    if ( subsel3dfld_ )
	envelope = subsel3dfld_->envelope();
}

void ui2D3DDataSelGrp::include3DCB( CallBacker* )
{
    if ( include3dfld_ )
	subsel3dfld_->setSensitive( include3dfld_->isChecked() );
}
