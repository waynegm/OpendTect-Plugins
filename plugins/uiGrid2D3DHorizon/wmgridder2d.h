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

class TaskRunner;
class TrcKeySampling;
namespace EM { class Horizon3D; }

typedef std::pair<double, double> TPoint;

class wmGridder2D
{ mODTextTranslationClass(wmGridder2D);
public:
    enum ScopeType   { BoundingBox, ConvexHull, Polygon };
    static const char*  ScopeNames[];
    static const char*  MethodNames[];
    static wmGridder2D* create(const char* methodName);
    
    virtual         ~wmGridder2D();

    void            setSearchRadius(float r) { searchradius_ = r; }
    float           getSearchRadius() const { return searchradius_; }
    
    virtual void    setPoint(const Coord& loc, const float val);
    uiString        infoMsg() const	{ return infomsg_; }
//    virtual bool	fillPar(IOPar&) const;
    virtual bool    usePar(const IOPar&);
    
    TrcKeySampling  getTrcKeySampling() const;
    bool            loadData();
    bool            setScope();
    void            getHorRange(Interval<int>&, Interval<int>&);
    bool            saveGridTo(EM::Horizon3D*);
    virtual bool    grid();
//    void addPolyMask( ODPolygon poly, bool outside=true );
//    void getConvexHull( ODPolygon& poly );
//    void getConcaveHull( ODPolygon& poly );
//    void maskGrid();
    
    static const char*  sKeyRowStep();
    static const char*  sKeyColStep();
    static const char*  sKeyMethod();
    static const char*  sKeyPolyScopeNodeNr(); 
    static const char*  sKeyPolyScopeNode(); 
    static const char*  sKeySearchRadius(); 
    static const char*  sKeyScopeType();
    static const char*  sKeyFaultID();
    static const char*  sKeyFaultNr();
    static const char*  sKey2DHorizonID();
    static const char*  sKey2DLineIDNr();
    static const char*  sKey2DLineID();
    static const char*  sKey3DHorizonID();
    
protected:
    
    wmGridder2D();
    
    MultiID                 hor2DID_;
    TypeSet<Pos::GeomID>    geomids_;
    MultiID                 hor3DID_;
    TrcKeySampling          hor3Dsubsel_;
    TypeSet<MultiID>        faultids_;
    ScopeType               scope_;
    ODPolygon<double>*      scopepoly_;
    std::vector<float>      vals_;
    std::vector<TPoint>     locs_; 
    kdbush::KDBush<TPoint, std::size_t>  kdtree_;
    Interval<float>         inlrg_;
    Interval<float>         crlrg_;
    float                   searchradius_;
    
    Array2DImpl<float>      grid_;
    TrcKeySampling          hs_;
    
    uiString                infomsg_;
    
    virtual void            interpolate_( const int idx, const int idy ) {}
    
};


class wmIDWGridder2D : public wmGridder2D
{ mODTextTranslationClass(wmIDWGridder2D);
public:
    wmIDWGridder2D();
    
    void            setPower(float p ) { pow_ = p; }
    float           getPower() const   { return pow_; }
    
    virtual bool    usePar(const IOPar&);
    
    static const char* sKeyPower();
    
    
protected:
    float   pow_;
    
    void    interpolate_( const int idx, const int idy );
    
};
#endif
