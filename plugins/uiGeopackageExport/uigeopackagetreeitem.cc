/*
*   uiGeopackageExport Plugin
*   Copyright (C) 2019  Wayne Mogg
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program. If not, see <http://www.gnu.org/licenses/>.
*/ 
#include "uigeopackagetreeitem.h"
#include "uigeopackagereader.h"

#include "file.h"
#include "filepath.h"
#include "oddirs.h"

#include "uisellinest.h"
#include "uicombobox.h"
#include "uifileinput.h"
#include "uispinbox.h"
#include "uilabel.h"
#include "uidialog.h"
#include "uistrings.h"
#include "uiodmain.h"
#include "uiodapplmgr.h"
#include "uiodscenemgr.h"
#include "uitreeview.h"
#include "uivispartserv.h"
#include "uimsg.h"

#include "viscoord.h"
#include "vishorizondisplay.h"
#include "visdrawstyle.h"
#include "vismaterial.h"
#include "vispolyline.h"
#include "vistransform.h"
#include "emhorizon3d.h"
#include "emmanager.h"
#include "interpol2d.h"
#include "zaxistransform.h"
#include "posidxpair2coord.h"

/*
 *  Calculate intersection points between a line and rectangular grid
 *  Gird assumed to start at the origin, endpoints are included in output
 * 
 */ 

bool makeLine( const Coord& start, const Coord& stop, const Coord& step, TypeSet<Coord>& output )
{
    output.erase();
    Coord dir = (stop-start);
    if (mIsZero(dir.abs(), mDefEpsF))
        return false;       // start and stop are the same
    float gridX = floor(start.x/step.x);
    float gridY = floor(start.y/step.y);
    float dgridX, dgridY, dtX, dtY;
	if (mIsZero(dir.x*dir.x, mDefEpsF))
		dir.x = dir.x < 0 ? -mDefEpsF : mDefEpsF;
	if (mIsZero(dir.y*dir.y, mDefEpsF))
		dir.y = dir.y < 0 ? -mDefEpsF : mDefEpsF;
	if (dir.x>0) {
        dgridX = 1;
        dtX = (gridX*step.x-start.x)/dir.x;
    } else {
        dgridX = -1;
        dtX = ((gridX-1)*step.x-start.x)/dir.x;
    }
    float ddtX = dgridX*step.x/dir.x;
    if (dir.y>0) {
        dgridY = 1;
        dtY = (gridY*step.y-start.y)/dir.y;
    } else {
        dgridY = -1;
        dtY = ((gridY-1)*step.y-start.y)/dir.y;
    }
    float ddtY = dgridY*step.y/dir.y;
    
    float t = 0.0;
	float tlast = -1.0;
    while (t<1.0) {
		if (t>=0 && t>tlast)
			output += start + dir * t;
		tlast = t;
        if (dtX<dtY) {
            t += dtX;
			dtY -= dtX;
			dtX = ddtX;
        } else {
            t += dtY;
            dtX -= dtY;
            dtY = ddtY;
        }
    }
    output += stop;
    return true;
}    

class Hor3DTool
{
public:
    Hor3DTool( const EM::Horizon3D* hor3d, const ZAxisTransform* ztran=0 );
    ~Hor3DTool();

    float interpZat( float inl, float crl, bool applZtransform=true ) const;
    void profile( const ODPolygon<Pos::Ordinate_Type> path, ManagedObjectSet<TypeSet<Coord3>>& prof, bool applZtransform=true );
    
protected:
    const EM::Horizon3D*    hor_;
    const ZAxisTransform*   ztrans_;
    StepInterval<float>     inlrg_;
    StepInterval<float>     crlrg_;
};

Hor3DTool::Hor3DTool( const EM::Horizon3D* hor3d, const ZAxisTransform* ztran )
: hor_(hor3d)
, ztrans_(ztran)
{
    TrcKeySampling range = hor3d->range();
    StepInterval<int> r = range.inlRange(); 
    inlrg_.set(r.start, r.stop, r.step);
    r = range.crlRange();
    crlrg_.set(r.start, r.stop, r.step);
    inlrg_.sort(); 
    crlrg_.sort();
}

Hor3DTool::~Hor3DTool()
{ }

float Hor3DTool::interpZat( float inl, float crl, bool applyZtransform ) const
{
    if (applyZtransform && !ztrans_)
        return mUdf(float);
    
    if (inlrg_.includes(inl, false) && crlrg_.includes(crl, false)) {
        Interpolate::LinearReg2D<float> interp;
        float fx = floor(inl/inlrg_.step)*inlrg_.step;
        float fy = floor(crl/crlrg_.step)*crlrg_.step;
        BinID b00(fx, fy);
        BinID b10 = b00+BinID(inlrg_.step,0);
        BinID b01 = b00+BinID(0,crlrg_.step);
        BinID b11 = b00+BinID(inlrg_.step,crlrg_.step);
        float z00 = hor_->getZ(b00);
        if (mIsUdf(z00))
            return mUdf(float);
        float z10 = hor_->getZ(b10);
        if (mIsUdf(z10))
            return mUdf(float);
        float z01 = hor_->getZ(b01);
        if (mIsUdf(z01))
            return mUdf(float);
        float z11 = hor_->getZ(b11);
        if (mIsUdf(z11))
            return mUdf(float);
        if (applyZtransform) {
            ztrans_->transform(b00, SamplingData<float>(z00,1), 1, &z00);
            ztrans_->transform(b10, SamplingData<float>(z10,1), 1, &z10);
            ztrans_->transform(b01, SamplingData<float>(z01,1), 1, &z01);
            ztrans_->transform(b11, SamplingData<float>(z11,1), 1, &z11);
        }
        interp.set(z00, z01, z10, z11);
        return interp.apply((inl-fx)/inlrg_.step, (crl-fy)/crlrg_.step);
    } else
        return mUdf(float);
}

void Hor3DTool::profile( const ODPolygon<Pos::Ordinate_Type> path, ManagedObjectSet<TypeSet<Coord3>>& prof, bool applyZtransform )
{
    prof.erase();
    TypeSet<Coord> line;
    TypeSet<Coord3>* seg = 0;
    for (int iv=(path.isClosed()?0:1); iv<path.size(); iv++) {
        Coord p1 = SI().binID2Coord().transformBackNoSnap(path.prevVertex(iv));
        Coord p2 = SI().binID2Coord().transformBackNoSnap(path.getVertex(iv));
        if (makeLine(p1, p2, Coord(inlrg_.step, crlrg_.step), line)) {
            for (int il=0; il<line.size(); il++) {
                float zval = interpZat(line[il].x, line[il].y, applyZtransform);
                if (mIsUdf(zval)) {
                    seg = 0;
                } else {
                    if (!seg) {
                        seg = new TypeSet<Coord3>;
                        prof.add(seg);
                    }
                    *seg += Coord3(line[il], zval);
                }
            }
        }
    }
}

class uiGeopackageParsDlg : public uiDialog
{ mODTextTranslationClass(uiGeopackageParsDlg);
public:
    uiGeopackageParsDlg( uiParent* p, uiGeopackageReader& reader, const OD::LineStyle& ls, const int shift=0 )
        : uiDialog(p, Setup(tr("Geopackage Display Options"),mNoDlgTitle,mTODOHelpKey))
        , reader_(reader)
    {
        BufferString defseldir = FilePath(GetDataDir()).add("Misc").fullPath();
        filefld_ = new uiFileInput( this, "Input file",
                                    uiFileInput::Setup(uiFileDialog::Gen)
                                    .forread(true).filter("*.gpkg").defseldir(defseldir).allowallextensions(false) );
        filefld_->setDefaultExtension( "gpkg" );
        if (reader_.fileName())
            filefld_->setFileName(reader_.fileName());
        filefld_->valuechanged.notify(mCB(this, uiGeopackageParsDlg, fileChgCB));
        
        layerfld_ = new uiComboBox(this, "Layer");
        layerfld_->attach (alignedBelow, filefld_);
        layerfld_->setReadOnly(true);
        layerfld_->selectionChanged.notify(mCB(this, uiGeopackageParsDlg, layerChgCB));
        
		shiftfld_ = new uiLabeledSpinBox(this, tr("Float layer above by"));
		shiftfld_->box()->setInterval(-10, 10, 1);
		shiftfld_->box()->setValue(shift);
		shiftfld_->attach(alignedBelow, layerfld_);

		uiLabel* lbl = new uiLabel(this, SI().zDomain().uiUnitStr(true));
		lbl->attach(rightOf, shiftfld_);

        uiSelLineStyle::Setup lssu; 
        lssu.drawstyle(false);
        lsfld_ = new uiSelLineStyle( this, ls, lssu );
        lsfld_->attach(alignedBelow, shiftfld_);
        
        setOkCancelText( uiStrings::sApply(), uiStrings::sClose() );
        fileChgCB(0);
    }

    const OD::LineStyle& getLineStyle() const { return lsfld_->getStyle(); }
	int		getShift() const { return shiftfld_->box()->getIntValue();  }

protected:
    bool acceptOK( CallBacker* )
    {
        return true;
    }
    
    void fillLayers()
    {
        if (layerfld_) {
            BufferStringSet layerNames;
            reader_.getLayers(layerNames);
			layerfld_->setEmpty();
            layerfld_->addItems(layerNames);
        }
    }
    
    void fileChgCB( CallBacker* )
    {
        if (reader_.open(filefld_->fileName())) {
            fillLayers();
            if (reader_.layerName())
                layerfld_->setCurrentItem(reader_.layerName());
            else
                layerfld_->setCurrentItem(0);
        } else {
            layerfld_->setEmpty();
        }
        layerChgCB(0);
    }
    
    void layerChgCB( CallBacker* )
    {
        reader_.setLayer(layerfld_->text());
    }
    
    uiFileInput*                    filefld_;
    uiComboBox*                     layerfld_;
	uiLabeledSpinBox*				shiftfld_;
	uiSelLineStyle*                 lsfld_;
    uiGeopackageReader&             reader_;
};
    
const char* uiGeopackageTreeItem::sKeyGeopackageDefString(){ return "Geopackage Display";}
const char* uiGeopackageTreeItem::sKeyGeopkg() { return "Geopkg";}
const char* uiGeopackageTreeItem::sKeyGeopkgNr() { return "Nr Geopkg";}
const char* uiGeopackageTreeItem::sKeyGeopkgStart() { return "Start";}

void uiGeopackageTreeItem::initClass()
{
    uiODDataTreeItem::factory().addCreator( create, 0 ); 
}

uiGeopackageTreeItem::uiGeopackageTreeItem( const char* parenttype )
    : uiODDataTreeItem(parenttype)
    , optionsmenuitem_(m3Dots(uiStrings::sProperties()))
    , linewidth_(1)
    , color_(0,0,0)
    , material_(0)
    , drawstyle_(0)
    , lines_(0)
    , zshift_(0)
{
    reader_ = new uiGeopackageReader();
    optionsmenuitem_.iconfnm = "disppars";
    
    mAttachCB( ODMainWin()->sessionSave, uiGeopackageTreeItem::sessionSaveCB );
    mAttachCB( ODMainWin()->sessionRestore, uiGeopackageTreeItem::sessionRestoreCB );
}

uiGeopackageTreeItem::~uiGeopackageTreeItem()
{
    if (reader_)
        delete reader_;
    
    visSurvey::HorizonDisplay* hordisp = getHorDisp();
    if ( hordisp )
        hordisp->getMovementNotifier()->remove(mCB(this,uiGeopackageTreeItem,checkCB));
        
    ODMainWin()->applMgr().visServer()->removeAllNotifier().remove(mCB(this,uiGeopackageTreeItem,visClosingCB) );
        
    if ( !parent_ )
            return;
        
    parent_->checkStatusChange()->remove(mCB(this,uiGeopackageTreeItem,checkCB));
    detachAllNotifiers();
}

void uiGeopackageTreeItem::prepareForShutdown()
{
    visClosingCB( 0 );
}

void uiGeopackageTreeItem::visClosingCB( CallBacker* )
{
    removeAll();
}

uiString uiGeopackageTreeItem::createDisplayName() const
{
    FilePath fp(reader_->fileName());
    return tr( "Geopkg: %1: %2" ).arg(fp.baseName()).arg(reader_->layerName());
}

bool uiGeopackageTreeItem::init()
{
    if ( !uiODDataTreeItem::init() )
        return false;
    
    uitreeviewitem_->setChecked( true );
    parent_->checkStatusChange()->notify( mCB(this,uiGeopackageTreeItem,checkCB) );
    
    ODMainWin()->applMgr().visServer()->removeAllNotifier().notify(mCB(this,uiGeopackageTreeItem,visClosingCB) );
    
    return true;
}

uiODDataTreeItem* uiGeopackageTreeItem::create( const Attrib::SelSpec& as, const char* parenttype )
{
    BufferString defstr = as.defString();
    if ( defstr != sKeyGeopackageDefString() )
	return 0;

    
    uiGeopackageTreeItem* gpitem = new uiGeopackageTreeItem(parenttype);
    return gpitem ? (uiODDataTreeItem*) gpitem : 0;
}

void uiGeopackageTreeItem::checkCB( CallBacker* )
{
    bool newstatus = uitreeviewitem_->isChecked();
    if ( newstatus && parent_ )
        newstatus = parent_->isChecked();
    
    visSurvey::HorizonDisplay* hordisp = getHorDisp();
    if ( !hordisp ) return;
    
    const bool display = newstatus && hordisp && !hordisp->displayedOnlyAtSections();
    
    if ( lines_ ) 
        lines_->turnOn( display );
    
}

bool uiGeopackageTreeItem::doubleClick( uiTreeViewItem* item )
{
    if ( item != uitreeviewitem_ )
        return uiTreeItem::doubleClick( item );
    
    if ( !select() ) return false;
    
    showPropertyDlg();
    return true;
}

void uiGeopackageTreeItem::removeAll()
{
    if ( lines_ )
    {
        applMgr()->visServer()->removeObject( lines_, sceneID() );
        if (lines_) {
            lines_->unRef();
            lines_ = 0;
        }
    }
    if (drawstyle_) {
        drawstyle_->unRef();
        drawstyle_ = 0;
    }
    if (material_) {
        material_->unRef();
        material_ = 0;
    }
}

void uiGeopackageTreeItem::createMenu( MenuHandler* menu, bool istb )
{
    uiODDataTreeItem::createMenu( menu, istb );
    mAddMenuOrTBItem( istb, menu, &displaymnuitem_, &optionsmenuitem_, true, false );
}

void uiGeopackageTreeItem::handleMenuCB( CallBacker* cb )
{
    uiODDataTreeItem::handleMenuCB( cb );
    mCBCapsuleUnpackWithCaller( int, mnuid, caller, cb );
    mDynamicCastGet(MenuHandler*,menu,caller);
    
    if ( mnuid==-1 || menu->isHandled() )
        return;
    
    if ( mnuid==optionsmenuitem_.id )
    {
        menu->setIsHandled( true );
        showPropertyDlg();
    }
}

void uiGeopackageTreeItem::showPropertyDlg()
{
    OD::LineStyle ls( OD::LineStyle::Solid, linewidth_, color_ );
    uiGeopackageParsDlg propdlg( ODMainWin(), *reader_, ls, zshift_ );
    if (!propdlg.go())
        return;
    
	zshift_ = propdlg.getShift();
    OD::LineStyle nls = propdlg.getLineStyle();
    if (!drawstyle_) {
        drawstyle_ = new visBase::DrawStyle;
        drawstyle_->ref();
    }
    drawstyle_->setLineStyle( nls );
    if (!material_) {
        material_ = new visBase::Material;
        material_->ref();
    }
    material_->setColor( nls.color_ );
    color_ = nls.color_;
    linewidth_ = nls.width_;
    
    setName(createDisplayName());
    updateColumnText(uiODSceneMgr::cNameColumn());
    updateColumnText(uiODSceneMgr::cColorColumn());
    showLayer();
}

bool uiGeopackageTreeItem::createPolyLines()
{
    if ( lines_ ) {
        removeOldLinesFromScene();
        return true;
    }
    
    if ( ( lines_ = visBase::PolyLine::create() ) == 0 )
        return false;

    lines_->ref();
    lines_->setPickable( false, false );
    applMgr()->visServer()->addObject( lines_, sceneID(), false );
    
    OD::LineStyle ls( OD::LineStyle::Solid, linewidth_, color_ );
    if ( !drawstyle_ ) {
        drawstyle_ = new visBase::DrawStyle;
        drawstyle_->ref();
    }
    drawstyle_->setLineStyle( ls );
    lines_->addNodeState(drawstyle_);
    if ( !material_ ) {
        material_ = new visBase::Material;
        material_->ref();
        material_->setColor( color_ );
    }
	lines_->setMaterial(material_);

    return true;
}

void uiGeopackageTreeItem::showLayer()
{
    if (!createPolyLines())
        return;
    BufferString errmsg;
    if (reader_ && !reader_->isSameCRS(errmsg)) {
        ErrMsg(errmsg);
        if (!uiMSG().askGoOn(tr("Mismatch in project and file CRS, results may be unpredictable, proceed?")))
            return;
    }

    if (reader_) {
        uiVisPartServer* visserv = applMgr()->visServer();
        EM::ObjectID emid = getHorDisp()->getObjectID();
        mDynamicCastGet(EM::Horizon3D*,hor3d,EM::EMM().getObject(emid));
        if (!hor3d)
            return;
        mDynamicCastGet(visSurvey::Scene*,scene, visserv->getObject(sceneID()));
        ZAxisTransform* ztransform = scene ? scene->getZAxisTransform() : 0;

	float zshift = zshift_ / (float) scene->zDomainUserFactor();
        
        Hor3DTool h3t(hor3d, ztransform);
        ManagedObjectSet<ODPolygon<Pos::Ordinate_Type>> polys;
        removeOldLinesFromScene();
        while (reader_->getNextFeature(polys)) {
            ManagedObjectSet<TypeSet<Coord3>> profs;
            for (int ip=0; ip<polys.size(); ip++) {
                h3t.profile(*polys[ip], profs, (ztransform!=0));
            }
            for (int ip=0; ip<profs.size(); ip++) {
                TypeSet<Coord3>& seg = *profs[ip];
                Geometry::RangePrimitiveSet* ps = Geometry::RangePrimitiveSet::create();
                int start = lines_->size();
                for (int iv=0; iv<seg.size();iv++) {
                    Coord3 vrtxcoord = seg[iv];
					vrtxcoord.coord() = SI().binID2Coord().transform(seg[iv].coord());
					vrtxcoord.z -= zshift;
                    lines_->addPoint(vrtxcoord);
                }
                ps->setRange(Interval<int>(start,lines_->size()-1));
                lines_->addPrimitiveSet(ps);
            }
        }
		lines_->dirtyCoordinates();
    }
}

void uiGeopackageTreeItem::removeOldLinesFromScene()
{
    if ( lines_ )
        lines_->removeAllPrimitiveSets();
}

visSurvey::HorizonDisplay* uiGeopackageTreeItem::getHorDisp() const
{
    uiVisPartServer* visserv = applMgr()->visServer();
    mDynamicCastGet(visSurvey::HorizonDisplay*,hordisp,visserv->getObject(displayID()))
    return hordisp;
}

void uiGeopackageTreeItem::updateColumnText( int col )
{
    if (col == uiODSceneMgr::cColorColumn())
        uitreeviewitem_->setPixmap(uiODSceneMgr::cColorColumn(), color_);
    else if (col == uiODSceneMgr::cNameColumn())
	uiODDataTreeItem::updateColumnText( col );
    
    uiVisPartServer* visserv = applMgr()->visServer();
    visSurvey::HorizonDisplay* hordisp = getHorDisp();
    if ( !hordisp ) return;
                   
    if (lines_) {
        const bool solomode = visserv->isSoloMode();
        const bool turnon = !hordisp->displayedOnlyAtSections() && ( (solomode && hordisp->isOn()) || (!solomode && hordisp->isOn() && isChecked()) );
        lines_->turnOn( turnon );
    }
}

void uiGeopackageTreeItem::sessionRestoreCB( CallBacker* )
{
    IOPar& iop = ODMainWin()->sessionPars();
    usePar(iop);
}

void uiGeopackageTreeItem::sessionSaveCB( CallBacker* )
{
    IOPar& iop = ODMainWin()->sessionPars();
    fillPar(iop);
}

bool uiGeopackageTreeItem::fillPar(IOPar& par) const
{
    int nrGP = 1;
    const MultiID mid = getHorDisp()->getMultiID();
    BufferString nrkey = sKeyGeopkg();
    BufferString id;
    id.add(mid.buf()).replace(".","_");
    nrkey.add(".").add(id).add(".");
    BufferString startkey = nrkey;
    startkey.add(sKeyGeopkgStart());
    nrkey.add(sKeyGeopkgNr());
    if (par.get(nrkey, nrGP))
	nrGP++;
    IOPar gppar;
    gppar.set(sKey::FileName(), reader_->fileName());
    gppar.set(sKey::Target(), reader_->layerName());
    gppar.set(sKey::Color(), color_);
    gppar.set(sKey::Weight(), linewidth_);
    
    BufferString key = sKeyGeopkg();
    key.add(".").add(id).add(".").add(nrGP);
    par.set(nrkey, nrGP);
    par.set(startkey, 1);
    par.mergeComp(gppar, key);
    return true;
}

void uiGeopackageTreeItem::usePar(IOPar& par)
{
    visSurvey::HorizonDisplay* hdisp = getHorDisp();
    int nrGP = 0;
    if ( hdisp )
    {
	const MultiID mid = hdisp->getMultiID();
	BufferString nrkey = sKeyGeopkg();
	BufferString id;
	id.add(mid.buf()).replace(".","_");
	nrkey.add(".").add(id).add(".");
	BufferString startkey = nrkey;
	startkey.add(sKeyGeopkgStart());
	nrkey.add(sKeyGeopkgNr());
	int idx;
	if ( par.get(nrkey, nrGP) && par.get(startkey, idx) && nrGP>0) {
	    BufferString key = sKeyGeopkg();
	    key.add(".").add(id).add(".").add(idx);
	    IOPar* gpar = par.subselect(key);
	    if (gpar) {
		BufferString filename, layername;
		gpar->get(sKey::FileName(), filename);
		gpar->get(sKey::Target(), layername);
		gpar->get(sKey::Color(), color_);
		gpar->get(sKey::Weight(), linewidth_);
		if ( reader_->open(filename) ) {
		    reader_->setLayer(layername);
		    showLayer();
		}
	    }
	    par.removeSubSelection(key);
	    nrGP--;
	    idx++;
	    if ( nrGP > 0 ) {
		par.set(nrkey, nrGP);
		par.set(startkey, idx);
	    } else {
		par.removeSubSelection(nrkey);
		par.removeSubSelection(startkey);
	    }
	}
    }
}
