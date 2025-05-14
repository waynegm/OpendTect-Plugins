#include "wmgridder2d.h"
#include "mbagridder2d.h"
#include "idwgridder2d.h"
#include "ltpsgridder2d.h"
#include "nrngridder2d.h"

#include "uimsg.h"
#include "bufstring.h"
#include "keystrs.h"
#include "uistring.h"
#include "survinfo.h"
#include "posidxpair2coord.h"
#include "math2.h"
#include "emmanager.h"
#include "emobject.h"
#include "emhorizon2d.h"
#include "emhorizon3d.h"
#include "emioobjinfo.h"
#include "survgeom2d.h"
#include "posinfo2d.h"
#include "ioman.h"
#include "picklocation.h"
#include "pickset.h"
#include "picksettr.h"
#include "uitaskrunner.h"


const char* wmGridder2D::sKeyInput()		{ return "Input"; }
const char* wmGridder2D::sKeyGridDef()		{ return "GridDef"; }
const char* wmGridder2D::sKeyScopeHorID()   { return "ScopeHorID"; }
const char* wmGridder2D::sKeyMethod()       { return "Method"; }
const char* wmGridder2D::sKeyClipPolyID()   { return "ClipPolyID"; }
const char* wmGridder2D::sKeySearchRadius() { return "SearchRadius"; }
const char* wmGridder2D::sKeyMaxPoints()    { return "MaxPoints"; }
const char* wmGridder2D::sKeyScopeType()    { return "ScopeType"; }
const char* wmGridder2D::sKeyContourPolyID()  { return "ContourPolyID"; }
const char* wmGridder2D::sKeyContourPolyNr()  { return "ContourPolyNr"; }
const char* wmGridder2D::sKeyFaultPolyID()  { return "FaultPolyID"; }
const char* wmGridder2D::sKeyFaultPolyNr()  { return "FaultPolyNr"; }
const char* wmGridder2D::sKeyFaultID()      { return "FaultID"; }
const char* wmGridder2D::sKeyFaultNr()      { return "FaultNr"; }
const char* wmGridder2D::sKey2DHorizonID()  { return "2DHorizonID"; }
const char* wmGridder2D::sKey2DLineIDNr()   { return "2DLineIDNr"; }
const char* wmGridder2D::sKey2DLineID()     { return "2DLineID"; }
const char* wmGridder2D::sKey3DHorizonID()  { return "3DHorizonID"; }
const char* wmGridder2D::sKeyRegularization()    { return "Regularization"; }
const char* wmGridder2D::sKeyTension()      { return "Tension"; }

const char* wmGridder2D::MethodNames[] =
{
    "Local Thin-plate Spline",
    "Multilevel B-Splines",
    "Inverse Distance Weighted",
    "Nearest Neighbour",
    0
};

const char* wmGridder2D::ScopeNames[] =
{
    "Range",
    "Bounding Box",
    "Convex Hull",
    "Horizon",
    0
};

wmGridder2D* wmGridder2D::create( const char* methodName )
{
    BufferString tmp(methodName);
    if (tmp == MethodNames[LTPS])
	return (wmGridder2D*) new wmLTPSGridder2D();
    else if (tmp == MethodNames[IDW])
	return (wmGridder2D*) new wmIDWGridder2D();
    else if (tmp == MethodNames[MBA])
	return (wmGridder2D*) new wmMBAGridder2D();
    else if (tmp == MethodNames[NRN])
	return (wmGridder2D*) new wmNRNGridder2D();
    else {
        ErrMsg("wmGridder2D::create - unrecognised method name");
        return nullptr;
    }
}

bool wmGridder2D::canHandleFaultPolygons( const char* methodName )
{
    BufferString tmp(methodName);
    if (tmp == MethodNames[LTPS])
	return true;
    else if (tmp == MethodNames[IDW])
	return true;
    else if (tmp == MethodNames[MBA])
	return true;
    else if (tmp == MethodNames[NRN])
	return true;
    else {
	ErrMsg("wmGridder2D::canHandleFaultPolygons - unrecognised method name");
	return false;
    }
}

wmGridder2D::wmGridder2D()
    : searchradius_(mUdf(float))
    , maxpoints_(mUdf(int))
{
    hs_ = SI().sampling( false ).hsamp_;
    hor2DID_.setUdf();
    hor3DID_.setUdf();
    croppolyID_.setUdf();
    horScopeID_.setUdf();
    inlrg_.setUdf();
    crlrg_.setUdf();
}

wmGridder2D::~wmGridder2D()
{
    deleteAndNullPtr(carr_);
}

// loc is in survey grid coordinates
void wmGridder2D::setPoint( const Coord& binLoc, const float val )
{
    binLocs_ += binLoc;
    vals_ += val;
    includeInRange(binLoc);
}

void wmGridder2D::includeInRange(const Coord& pos)
{
    if (inlrg_.isUdf()) {
	inlrg_.set(pos.x_,pos.x_);
	crlrg_.set(pos.y_,pos.y_);
    } else {
	inlrg_.include(pos.x_);
	crlrg_.include( pos.y_ );
    }
}

bool wmGridder2D::usePar(const IOPar& par)
{
    PtrMan<IOPar> inp_par = par.subselect(sKey::Input());
    if (!inp_par)
	return false;
    hor2DID_.setUdf();
    geomids_.erase();
    if (inp_par->get(sKey2DHorizonID(), hor2DID_)) {
        int nlines=0;
        inp_par->get(sKey2DLineIDNr(), nlines);
        if (nlines>0) {
            for (int idx=0; idx<nlines; idx++) {
                Pos::GeomID id;
                if (inp_par->get(IOPar::compKey(sKey2DLineID(),idx), id))
                    geomids_ += id;
            }
        }
    }

    hor3DID_.setUdf();
    if (inp_par->get(sKey3DHorizonID(), hor3DID_))
        hor3Dsubsel_.usePar(*inp_par);

    PtrMan<IOPar> grd_par = par.subselect(sKeyGridDef());
    if (!grd_par)
	return false;
    int scopetype;
    grd_par->get(sKeyScopeType(),scopetype);
    scope_ = (ScopeType)scopetype;

    horScopeID_.setUdf();
    grd_par->get(sKeyScopeHorID(), horScopeID_);

    croppolyID_.setUdf();
    grd_par->get(sKeyClipPolyID(), croppolyID_);

    int nrfaultpoly = 0;
    faultpolyID_.erase();
    if (grd_par->get(sKeyFaultPolyNr(), nrfaultpoly)) {
        for (int idx=0; idx<nrfaultpoly; idx++) {
            MultiID id;
            if (!grd_par->get(IOPar::compKey(sKeyFaultPolyID(), idx), id))
                return false;
            faultpolyID_ += id;
        }
    }

    int nrcontpoly = 0;
    contpolyID_.erase();
    if (grd_par->get(sKeyContourPolyNr(), nrcontpoly)) {
	for (int idx=0; idx<nrcontpoly; idx++) {
	    MultiID id;
	    if (!grd_par->get(IOPar::compKey(sKeyContourPolyID(), idx), id))
		return false;
	    contpolyID_ += id;
	}
    }


    grd_par->get(sKeySearchRadius(), searchradius_);
    grd_par->get(sKeyMaxPoints(), maxpoints_);

    hs_.usePar(*grd_par);

    faultids_.erase();
    int nrfaults = 0;
    if (grd_par->get(sKeyFaultNr(), nrfaults)) {
        for (int idx=0; idx<nrfaults; idx++) {
            MultiID fltid;
            if (!grd_par->get(IOPar::compKey(sKeyFaultID(),idx),fltid))
                return false;
            faultids_ += fltid;
        }
    }
    return true;
}

TrcKeySampling wmGridder2D::getTrcKeySampling() const
{
    return hs_;
}

void wmGridder2D::getHorRange(Interval<int>& inlrg, Interval<int>& crlrg)
{
    inlrg = Interval<int>(hs_.inlRange().start_, hs_.inlRange().stop_);
    crlrg = Interval<int>(hs_.crlRange().start_, hs_.crlRange().stop_);
}

bool wmGridder2D::saveGridTo(EM::Horizon3D* hor3d)
{
	hor3d->geometry().geometryElement()->setArray(hs_.start_, hs_.step_, grid_, true);
    return true;
}

bool wmGridder2D::loadData()
{
    cvxhullpoly_.erase();

    if (!hor3DID_.isUdf()) {
        EM::EMObject* obj = EM::EMM().loadIfNotFullyLoaded(hor3DID_);
        if (!obj) {
            ErrMsg("wmGridder2D::loadData - loading 3D horizon failed");
            return false;
        }
        obj->ref();
        mDynamicCastGet(EM::Horizon3D*,hor,obj);
        if (!hor) {
           ErrMsg("wmGridder2D::loadData - casting 3D horizon failed");
           obj->unRef();
           return false;
        }
        for (int iln=hor3Dsubsel_.start_.inl(); iln<=hor3Dsubsel_.stop_.inl(); iln+=hor3Dsubsel_.step_.inl()) {
	    Coord coord;
	    bool first = true;
            for (int xln=hor3Dsubsel_.start_.crl(); xln<=hor3Dsubsel_.stop_.crl(); xln+=hor3Dsubsel_.step_.crl()) {
                BinID bid(iln,xln);
                TrcKey tk(bid);
                const float z = hor->getZ( tk );
                if (mIsUdf(z))
                    continue;
		coord = Coord(iln, xln);
		setPoint(Coord(iln, xln), z);
		if (first) {
		    cvxhullpoly_.add(coord);
		    first = false;
		}
            }
            cvxhullpoly_.add(coord);
        }
        obj->unRef();
    }

    if (!hor2DID_.isUdf() && geomids_.size()>0) {
        EM::EMObject* obj = EM::EMM().loadIfNotFullyLoaded(hor2DID_);
        if (!obj) {
            ErrMsg("wmGridder2D::loadData - loading 2D horizon failed");
            return false;
        }
        obj->ref();
        mDynamicCastGet(EM::Horizon2D*,hor,obj);
        if (!hor) {
            ErrMsg("wmGridder2D - casting 2D horizon failed");
            obj->unRef();
            return false;
        }
        for (int idx=0; idx<geomids_.size(); idx++) {
            const StepInterval<int> trcrg = hor->geometry().colRange( geomids_[idx] );
            mDynamicCastGet(const Survey::Geometry2D*,survgeom2d,Survey::GM().getGeometry(geomids_[idx]))
            if (!survgeom2d || trcrg.isUdf() || !trcrg.step_)
                continue;

            TrcKey tk( geomids_[idx], -1 );
            float spnr = mUdf(float);
	    Coord binLoc;
	    bool first = true;
            for ( int trcnr=trcrg.start_; trcnr<=trcrg.stop_; trcnr+=trcrg.step_ ) {
                tk.setTrcNr( trcnr );
                const float z = hor->getZ( tk );
                if (mIsUdf(z))
                    continue;

		Coord coord;
		survgeom2d->getPosByTrcNr( trcnr, coord, spnr );
		binLoc = SI().binID2Coord().transformBackNoSnap(coord);
                setPoint(binLoc, z);
		if (first) {
		    cvxhullpoly_.add(binLoc);
		    first = false;
		}
            }
            cvxhullpoly_.add(binLoc);
        }
        obj->unRef();
    }

    ODPolygon<Pos::Ordinate_Type> poly;
    for (int idx=0; idx<contpolyID_.size(); idx++) {
	poly.erase();
	PtrMan<IOObj> ioobj = IOM().get(contpolyID_[idx]);
	if (!ioobj) {
	    ErrMsg("wmGridder2D::loadData - cannot get contour polyline ioobj");
	    return false;
	}
	RefMan<Pick::Set> ps = new Pick::Set;
	uiString msg;
	if (!PickSetTranslator::retrieve(*(ps.ptr()), ioobj.ptr(), true, msg)) {
	    BufferString tmp("wmGridder2D::loadData - error reading contour polygon - ");
	    tmp += msg.getString();
	    ErrMsg(tmp);
	    return false;
	}
	for (int idp=0; idp<ps->size(); idp++) {
	    const Pick::Location& pl = ps->get(idp);
	    Coord binLoc = SI().binID2Coord().transformBackNoSnap(pl.pos());
	    setPoint(binLoc, pl.z());
	    cvxhullpoly_.add(binLoc);
	}
    }

    if (!croppolyID_.isUdf()) {
        croppoly_.erase();
        PtrMan<IOObj> ioobj = IOM().get(croppolyID_);
        if (!ioobj) {
            ErrMsg("wmGridder2D::loadData - cannot get crop polygon ioobj");
            return false;
        }
	RefMan<Pick::Set> ps = new Pick::Set;
        uiString msg;
        if (!PickSetTranslator::retrieve(*(ps.ptr()), ioobj.ptr(), true, msg)) {
            BufferString tmp("wmGridder2D::loadData - error reading crop polygon - ");
            tmp += msg.getString();
            ErrMsg(tmp);
            return false;
        }
        for (int idx=0; idx<ps->size(); idx++) {
            const Pick::Location& pl = ps->get(idx);
	    Coord binLoc = SI().binID2Coord().transformBackNoSnap(pl.pos());
            croppoly_.add(binLoc);
        }
        croppoly_.setClosed(true);
    }

    faultpoly_.erase();
    for (int idx=0; idx<faultpolyID_.size(); idx++) {
        ODPolygon<Pos::Ordinate_Type>* fault = new ODPolygon<Pos::Ordinate_Type>();
        PtrMan<IOObj> ioobj = IOM().get(faultpolyID_[idx]);
        if (!ioobj) {
            ErrMsg("wmGridder2D::loadData - cannot get fault polygon ioobj");
            return false;
        }
	RefMan<Pick::Set> ps = new Pick::Set;
        uiString msg;
        if (!PickSetTranslator::retrieve(*(ps.ptr()), ioobj.ptr(), true, msg)) {
            BufferString tmp("wmGridder2D::loadData - error reading fault polygon - ");
            tmp += msg.getString();
            ErrMsg(tmp);
            return false;
        }
        for (int idp=0; idp<ps->size(); idp++) {
            const Pick::Location& pl = ps->get(idp);
	    Coord binLoc = SI().binID2Coord().transformBackNoSnap(pl.pos());
	    fault->add(binLoc);
        }
        if (ps->disp_.connect_==Pick::Set::Disp::Close)
          fault->setClosed( true );
        else
          fault->setClosed( false );
        faultpoly_ += fault;
    }
    return true;
}

bool wmGridder2D::setScope()
{
    if (scope_==BoundingBox || scope_==ConvexHull) {
        SI().snapStep(hs_.step_, BinID(-1,-1));
        BinID start((int)Math::Floor(inlrg_.start_), (int)Math::Floor(crlrg_.start_));
        BinID stop((int)Math::Floor(inlrg_.stop_), (int)Math::Floor(crlrg_.stop_));
        SI().snap(start, BinID(-1,-1));
        SI().snap(stop, BinID(1,1));
        StepInterval<int> inlrg(start.inl(), stop.inl(), hs_.step_.first);
        inlrg.snap(inlrg.stop_, OD::SnapUpward);
        StepInterval<int> crlrg(start.crl(), stop.crl(), hs_.step_.second);
        crlrg.snap(crlrg.stop_, OD::SnapUpward);
        hs_.set(inlrg, crlrg);
        if (scope_==ConvexHull )
            cvxhullpoly_.convexHull();
        else
            cvxhullpoly_.erase();
    } else if (scope_==Horizon && !horScopeID_.isUdf()) {
        EM::IOObjInfo eminfo(horScopeID_);
        hs_.setLineRange(eminfo.getInlRange());
        hs_.setTrcRange(eminfo.getCrlRange());
    }

    return true;
}

bool wmGridder2D::isUncropped( Coord pos ) const
{
    bool result = true;
    if (croppolyID_.isUdf()) {
	if ( scope_ == ConvexHull )
	    result = cvxhullpoly_.isInside( pos, true, mDefEpsD);
    } else if ( scope_==ConvexHull )
	result = croppoly_.isInside( pos, true, mDefEpsD) && cvxhullpoly_.isInside( pos, true, mDefEpsD);
    else
	result = croppoly_.isInside( pos, true, mDefEpsD);

    return result;
}

bool wmGridder2D::inFaultHeave(Coord pos) const
{
    for (int idx=0; idx<faultpoly_.size(); idx++) {
        if (faultpoly_[idx]->isClosed() && faultpoly_[idx]->isInside(pos, false, mDefEpsD))
            return true;
    }
    return false;
}

bool wmGridder2D::segmentsIntersect( Coord s1p1, Coord s1p2, Coord s2p1, Coord s2p2 )
{
    const double s1lx = s1p2.x_>=s1p1.x_ ? s1p1.x_ : s1p2.x_;
    const double s1rx = s1p2.x_>=s1p1.x_ ? s1p2.x_ : s1p1.x_;
    const double s1by = s1p2.y_>=s1p1.y_ ? s1p1.y_ : s1p2.y_;
    const double s1ty = s1p2.y_>=s1p1.y_ ? s1p2.y_ : s1p1.y_;
    const double s2lx = s2p2.x_>=s2p1.x_ ? s2p1.x_ : s2p2.x_;
    const double s2rx = s2p2.x_>=s2p1.x_ ? s2p2.x_ : s2p1.x_;
    const double s2by = s2p2.y_>=s2p1.y_ ? s2p1.y_ : s2p2.y_;
    const double s2ty = s2p2.y_>=s2p1.y_ ? s2p2.y_ : s2p1.y_;

//     Interval<Pos::Ordinate_Type> ix1(s1p2.x, s1p1.x);
//     Interval<Pos::Ordinate_Type> ix2(s2p2.x, s2p1.x);
//     Interval<Pos::Ordinate_Type> iy1(s1p2.y, s1p1.y);
//     Interval<Pos::Ordinate_Type> iy2(s2p2.y, s2p1.y);

//    if (ix1.overlaps(ix2) && iy1.overlaps(iy2)) {
    if ( s1rx<s2lx || s1lx>s2rx || s1ty<s2by || s1by>s2ty )
	return false;
    {
        double denom = ((s2p2.y_-s2p1.y_)*(s1p2.x_-s1p1.x_))-((s2p2.x_-s2p1.x_)*(s1p2.y_-s1p1.y_));
        if (mIsZero(denom, mDefEps))
            return false;
        double ua = ((s2p2.x_-s2p1.x_)*(s1p1.y_-s2p1.y_)-(s2p2.y_-s2p1.y_)*(s1p1.x_-s2p1.x_))/denom;
        double ub = ((s1p2.x_-s1p1.x_)*(s1p1.y_-s2p1.y_)-(s1p2.y_-s1p1.y_)*(s1p1.x_-s2p1.x_))/denom;
        return ((ua >= 0.0) && (ua <= 1.0) && (ub >= 0.0) && (ub <= 1.0));
    }
}

bool wmGridder2D::faultBetween(Coord s1p1, Coord s1p2) const
{
    Interval<Pos::Ordinate_Type> segx(s1p1.x_, s1p2.x_);
    Interval<Pos::Ordinate_Type> segy(s1p1.y_, s1p2.y_);
    for (int idx=0; idx<faultpoly_.size(); idx++) {
	const auto* poly = faultpoly_[idx];
	if (!poly->getRange(true).overlaps(segx) || !poly->getRange(false).overlaps(segy))
	    return false;
	Coord s2p1 = poly->getVertex(0);
        for (int iv=0; iv<poly->size(); iv++) {
            const Coord s2p2 = poly->nextVertex(iv);
            if (segmentsIntersect(s1p1, s1p2, s2p1, s2p2))
               return true;
	    s2p1 = s2p2;
        }
    }
    return false;
}

mDefParallelCalc2Pars( GridMask, od_static_tr("GridMasker","Mask grid"),
		       const wmGridder2D*, interp, Threads::Lock&, lock )
mDefParallelCalcBody(
const TrcKeySampling& hs_ = interp_->hs_;
Array2DImpl<float>* grid_ = interp_->grid_;
TypeSet<od_int64>& interpidx_ = const_cast<TypeSet<od_int64>&>(interp_->interpidx_);
,
Task::Control state = getState();
if (state==Task::Stop)
    break;
else if (state==Task::Pause) {
    while (getState()==Task::Pause)
	Threads::sleep(1);
}
BinID gridBid = hs_.atIndex(idx);
double x = gridBid.inl();
double y = gridBid.crl();
int ix = hs_.inlIdx(gridBid.inl());
int iy = hs_.crlIdx(gridBid.crl());
Coord gridPos(x, y);
if (!interp_->isUncropped(gridPos) || interp_->inFaultHeave(gridPos)) {
    Threads::Locker lckr( lock_ );
    grid_->set(ix, iy, mUdf(float));
} else {
    Threads::Locker lckr( lock_ );
    interpidx_ += idx;
}
, )

bool wmGridder2D::prepareForGridding(uiParent* p)
{
    if (!loadData())
	return false;
    if (!setScope())
	return false;
    if (!grid_) {
	grid_ = new Array2DImpl<float>(hs_.nrInl(), hs_.nrCrl());
	if (!grid_) {
	    ErrMsg("wmGridder2D::prepareForGridding - grid_ is not allocated.");
	    return false;
	}
    } else
	grid_->setSize(hs_.nrInl(), hs_.nrCrl());

    grid_->setAll(0.0);
    interpidx_.erase();
    Threads::Lock lock;
    uiTaskRunner uitr(p);
    uitr.setCaption(toUiString("Mask grid"));
    GridMask gmask( hs_.totalNr(), this, lock);
    uitr.execute(gmask);

    return true;
}

mDefParallelCalc2Pars( LocalInterpolator, od_static_tr("LocalInterpolator","Interpolate nearest"),
		       const wmGridder2D*, interp, Threads::Lock&, lock )
mDefParallelCalcBody(
const TypeSet<Coord>& locs_ = interp_->binLocs_;
const TypeSet<float>& vals_ = interp_->vals_;
const TrcKeySampling& hs_ = interp_->hs_;
Array2DImpl<float>* grid_ = interp_->grid_;
Array2DImpl<float>* carr_ = interp_->carr_;
,
Task::Control state = getState();
if (state==Task::Stop)
    break;
else if (state==Task::Pause) {
    while (getState()==Task::Pause)
	Threads::sleep(1);
}

const Coord pos(locs_[idx]);
const BinID bidSnap = hs_.getNearest(BinID(mNINT32(pos.x_), mNINT32(pos.y_)));
if (mIsEqual(pos.x_, bidSnap.inl(), mDefEps) && mIsEqual(pos.y_, bidSnap.crl(), mDefEps)) {
    const int ix = hs_.inlIdx(bidSnap.inl());
    const int iy = hs_.crlIdx(bidSnap.crl());
    if (ix<0 || ix>=hs_.nrInl() || iy<0 || iy>=hs_.nrCrl())
	continue;

    const float prev = grid_->get(ix,iy);
    if (mIsUdf(prev))
	continue;

    Threads::Locker lckr( lock_ );
    grid_->set(ix, iy, prev + vals_[idx]);
    carr_->set(ix, iy, carr_->get(ix, iy) + 1.0);
} else {
    BinID r[4];
    r[0] = pos.x_<bidSnap.inl() ? bidSnap-BinID(hs_.step_.inl(),0) : bidSnap;
    r[1] = r[0] + BinID(hs_.step_.inl(),0);
    r[2] = pos.y_<bidSnap.crl() ? r[0]-BinID(0,hs_.step_.crl()) : r[0];
    r[3] = r[2] + BinID(hs_.step_.inl(),0);
    for (int ir=0; ir<4; ir++) {
	const int ix = hs_.inlIdx(r[ir].inl());
	const int iy = hs_.crlIdx(r[ir].crl());
	if (ix<0 || ix>=hs_.nrInl() || iy<0 || iy>=hs_.nrCrl())
	    continue;

	const float prev = grid_->get(ix,iy);
	if (mIsUdf(prev))
	    continue;

	const Coord rpos(r[ir].inl(), r[ir].crl());
	if (interp_->faultBetween(pos, rpos))
	    continue;

	const double dist = rpos.sqHorDistTo(pos);
	const double wgt = tanh(dist)/dist;

	Threads::Locker lckr( lock_ );
	grid_->set(ix, iy, prev + wgt*vals_[idx]);
	carr_->set(ix, iy, carr_->get(ix, iy) + wgt);
    }
}
, )

void wmGridder2D::localInterp(uiParent* p, bool approximation)
{
    if (!grid_) {
	uiMSG().error(tr("wmGridder2D::localInterp - grid_ is not allocated."));
	return;
    }

    delete carr_;
    carr_ = new Array2DImpl<float>(hs_.nrInl(), hs_.nrCrl());
    if (!carr_) {
	ErrMsg("wmGridder2D::localInterp - carr_ is not allocated.");
	return;
    }
    carr_->setAll(0.0);

    Threads::Lock lock;
    uiTaskRunner uitr(p);
    uitr.setCaption(toUiString("Interpolate to grid"));
    LocalInterpolator interp( binLocs_.size(), this, lock);
    uitr.execute(interp);

    binLocs_.erase();
    vals_.erase();
    if (!approximation)
	interpidx_.erase();

    for (od_int64 idx=0; idx<hs_.totalNr(); idx++) {
	const BinID gridBid = hs_.atIndex(idx);
	const int ix = hs_.inlIdx(gridBid.inl());
	const int iy = hs_.crlIdx(gridBid.crl());
	const float gridval = grid_->get(ix,iy);
	if (mIsUdf(gridval))
	    continue;

	const float carr = carr_->get(ix,iy);
	if (carr!=0.0) {
	    const float val = gridval/carr;
	    grid_->set(ix, iy, val);
	    vals_ +=  val;
	    binLocs_ += Coord(gridBid.inl(), gridBid.crl());
	} else if (!approximation)
		interpidx_ += idx;

    }
}

