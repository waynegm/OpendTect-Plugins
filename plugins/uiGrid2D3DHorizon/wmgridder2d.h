#ifndef wmgridder2d_h
#define wmgridder2d_h

#include <cstddef>
#include <vector>
#include <utility>
#include "kdbush.h"
#include "factory.h"
#include "trckeysampling.h"
#include "polygon.h"
#include "arrayndimpl.h"
#include "multiid.h"
#include "paralleltask.h"

class TaskRunner;
class TrcKeySampling;
class LocalInterpolator;
namespace EM { class Horizon3D; }


typedef std::pair<double, double> TPoint;

class wmGridder2D : public ParallelTask
{ mODTextTranslationClass(wmGridder2D);
public:
    friend class LocalInterpolator;
    enum ScopeType   { BoundingBox, Horizon };
    enum Method { LocalCCTS, IDW }; 
    static const char*  ScopeNames[];
    static const char*  MethodNames[];
    static wmGridder2D* create(const char* methodName);
    
    virtual         ~wmGridder2D();

    void            setSearchRadius(float r) { searchradius_ = r; }
    float           getSearchRadius() const { return searchradius_; }
    
    virtual void    setPoint(const Coord& loc, const float val);
//    virtual bool	fillPar(IOPar&) const;
    virtual bool    usePar(const IOPar&);
 
    uiString        uiNrDoneText() const    { return "Gridded"; }
    od_int64        totalNr() const { return totalnr_; }
    
    TrcKeySampling  getTrcKeySampling() const;
    bool            loadData();
    bool            setScope();
    bool            isUncropped( Coord ) const;
    bool            inFaultHeave( Coord ) const;
    static bool     segmentsIntersect(Coord, Coord, Coord, Coord);
    bool            faultBetween( Coord, Coord) const;
    void            getHorRange(Interval<int>&, Interval<int>&);
    bool            saveGridTo(EM::Horizon3D*);
    
    static const char*  sKeyScopeHorID();
    static const char*  sKeyRowStep();
    static const char*  sKeyColStep();
    static const char*  sKeyMethod();
    static const char*  sKeyClipPolyID(); 
    static const char*  sKeySearchRadius(); 
    static const char*  sKeyScopeType();
    static const char*  sKeyFaultPolyID();
    static const char*  sKeyFaultPolyNr();
    static const char*  sKeyFaultID();
    static const char*  sKeyFaultNr();
    static const char*  sKey2DHorizonID();
    static const char*  sKey2DLineIDNr();
    static const char*  sKey2DLineID();
    static const char*  sKey3DHorizonID();
    static const char*  sKeyRegularization();
    static const char*  sKeyTension();
    
    od_int64            nrIterations() const { return totalnr_; }
    virtual bool        doWork( od_int64 start, od_int64 stop, int threadid );
    virtual bool        prepareForGridding( bool localInterp=false );
    
protected:
    
    wmGridder2D();
    
    MultiID                 hor2DID_;
    TypeSet<Pos::GeomID>    geomids_;
    MultiID                 hor3DID_;
    TrcKeySampling          hor3Dsubsel_;

    MultiID                         croppolyID_;
    ODPolygon<Pos::Ordinate_Type>   croppoly_;
    
    TypeSet<MultiID>                            faultpolyID_;
    ObjectSet<ODPolygon<Pos::Ordinate_Type>>    faultpoly_;
    
    TypeSet<MultiID>        faultids_;
    ScopeType               scope_;
    MultiID                 horScopeID_;
    std::vector<float>      vals_;
    std::vector<TPoint>     locs_; 
    kdbush::KDBush<TPoint, std::size_t>  kdtree_;
    Interval<float>         inlrg_;
    Interval<float>         crlrg_;
    float                   searchradius_;
    float                   regularization_;
    
    Array2DImpl<float>*     grid_;
    Array2DImpl<float>*     carr_;
    TrcKeySampling          hs_;
    
    od_int64                totalnr_;
    
    virtual void            interpolate_( int idx, int idy, Coord pos ) {}
    void                    localInterp();
    
};


class wmIDWGridder2D : public wmGridder2D
{ mODTextTranslationClass(wmIDWGridder2D);
public:
    wmIDWGridder2D();
    
    virtual bool    usePar(const IOPar&);
    virtual bool    prepareForGridding(bool);
    
protected:
    void    interpolate_( int idx, int idy, Coord pos );
    
};
/*
class wmLTPSGridder2D : public wmGridder2D
{ mODTextTranslationClass(wmLTPSGridder2D);
public:
    wmLTPSGridder2D();
    
    virtual bool    usePar(const IOPar&);
    virtual bool    prepareForGridding(bool);
    
protected:
    void    interpolate_( int idx, int idy, Coord pos );
};
*/
class wmLCCTSGridder2D : public wmGridder2D
{ mODTextTranslationClass(wmLCCTSGridder2D);
public:
    wmLCCTSGridder2D();
    
    virtual bool    usePar(const IOPar&);
    virtual bool    prepareForGridding(bool);
    
protected:
    void    interpolate_( int idx, int idy, Coord pos );
    double  basis_( const double r ) const;
    float   tension_;
    double  p0_, p1_;
    
};
/*
class wmIterGridder2D: public wmGridder2D
{ mODTextTranslationClass(wmIterGridder2D);
public:
    wmIterGridder2D();
    
    virtual bool    usePar(const IOPar&);
    virtual bool    prepareForGridding(bool);
    virtual bool    doWork( od_int64 start, od_int64 stop, int threadid );
protected:
    
};
*/
#endif
