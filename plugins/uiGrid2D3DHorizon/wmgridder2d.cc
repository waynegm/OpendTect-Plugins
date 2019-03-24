#include "wmgridder2d.h"

#include "survinfo.h"
#include "posidxpair2coord.h"
#include "math2.h"

typedef std::vector<std::size_t> TIds;

const char* wmGridder2D::sKeyRowStep()      { return "RowStep"; }
const char* wmGridder2D::sKeyColStep()      { return "ColStep"; }
const char* wmGridder2D::sKeyMethod()       { return "Method"; }
const char* wmGridder2D::sKeyPolyNode()     { return "PolyNode"; }
const char* wmGridder2D::sKeySearchRadius() { return "SearchRadius"; }
const char* wmGridder2D::sKeyScopeType()    { return "ScopeType"; }
const char* wmGridder2D::sKeyFaultID()      { return "FaultID"; }
const char* wmGridder2D::sKeyFaultNr()      { return "FaultNr"; }

const char* wmIDWGridder2D::sKeyPower()     { return "Power"; }


wmGridder2D::wmGridder2D() 
    : kdtree_(10)
    , searchradius_(mUdf(float))
    , grid_(0,0)
{}

void wmGridder2D::setPoint( const Coord& loc, const float val )
{
    Coord pos = SI().binID2Coord().transformBackNoSnap(loc);
    locs_.push_back(TPoint(loc.x, loc.y));
    inlrg_.include( pos.x );
    crlrg_.include( pos.y );
    vals_.push_back(val);
}


void wmGridder2D::setTrcKeySampling()
{
    hs_ = SI().sampling( false ).hsamp_;
    Interval<int> inlrg((int)Math::Floor(inlrg_.start), (int)Math::Ceil(inlrg_.stop));
    Interval<int> crlrg((int)Math::Floor(crlrg_.start), (int)Math::Ceil(crlrg_.stop));
    hs_.setInlRange(inlrg);
    hs_.setCrlRange(crlrg);
}

void wmGridder2D::setTrcKeySampling( const ODPolygon<double>& polyROI )
{
    hs_ = SI().sampling( false ).hsamp_;
    ODPolygon<float> tmp;
    for (int idx=0; idx<polyROI.size(); idx++) {
        Coord pos = SI().binID2Coord().transformBackNoSnap(Coord(polyROI.getVertex(idx)));
        tmp.add(Geom::Point2D<float>(pos.x,pos.y));
    }
    Interval<float> inlrg = tmp.getRange(true);
    Interval<float> crlrg = tmp.getRange(false);
    hs_.setInlRange(Interval<int>((int) floor(inlrg.start), (int) ceil(inlrg.stop)));
    hs_.setCrlRange(Interval<int>((int) floor(crlrg.start), (int) ceil(crlrg.stop)));
}
void wmGridder2D::setTrcKeySampling( const TrcKeySampling& hs )
{
    hs_ = hs;
}

bool wmGridder2D::grid( TaskRunner* tr )
{
    grid_.setSize( hs_.nrInl(), hs_.nrCrl() );
    grid_.setAll(mUdf(float));
    
    if (!mIsUdf(searchradius_))
        kdtree_.fill(locs_);
    
    for( int idx=0; idx<hs_.nrInl(); idx++ ) {
        for (int idy=0; idy < hs_.nrCrl(); idy++ ) {
            interpolate_( idx, idy );
        }
    }
    return true;
//    applyPolyMasks();
}

wmIDWGridder2D::wmIDWGridder2D()
    : pow_(2.0)
{}

void wmIDWGridder2D::interpolate_( const int ix, const int iy )
{
    const Pos::IdxPair2Coord& b2c = SI().binID2Coord();
    const BinID bid = hs_.atIndex(ix,iy);
    const Coord pos = b2c.transform(bid);
    double val = 0.0;
    double wgtsum = 0.0;
    if (!mIsUdf(searchradius_)) {
        TIds result;
        kdtree_.range( pos.x-searchradius_, pos.y-searchradius_, pos.x+searchradius_, pos.y+searchradius_, [&result](const std::size_t id) { result.push_back(id); });
        if (result.size() == 0)
            return;
        double r2 = searchradius_*searchradius_;
        for (int idx=0; idx<=result.size(); idx++) {
            TPoint locs = locs_[result[idx]];
            double vals = vals_[result[idx]];
            double d = (locs.first-pos.x)*(locs.first-pos.x)+(locs.second-pos.y)*(locs.second-pos.y);
            if ( mIsZero(d,mDefEpsF)) {
                grid_.set(ix, iy, vals);
                return;
            }
            if (d<=r2) {
                double wgt = Math::PowerOf(d, 0.5*pow_);
                val += vals * wgt;
                wgtsum += wgt;
            }
        }
    } else {
        for (int idx=0; idx<locs_.size(); idx++) {
            double d = (locs_[idx].first-pos.x)*(locs_[idx].first-pos.x)+(locs_[idx].second-pos.y)*(locs_[idx].second-pos.y);
            if ( mIsZero(d,mDefEpsF)) {
                grid_.set(ix, iy, vals_[idx]);
                return;
            }
            double wgt = Math::PowerOf(d, 0.5*pow_);
            val += vals_[idx] * wgt;
            wgtsum += wgt;
        }
    }
    val /= wgtsum;
    grid_.set(ix, iy, (float) val);
}
