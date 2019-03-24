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

class TaskRunner;
class TrcKeySampling;


typedef std::pair<double, double> TPoint;

class wmGridder2D
{ mODTextTranslationClass(wmGridder2D);
public:
    mDefineFactoryInClass(wmGridder2D,factory);
    enum ScopeType   { BoundingBox, ConvexHull, Polygon };
    
    virtual         ~wmGridder2D();

    void            setSearchRadius(float);
    float           getSearchRadius() const { return searchradius_; }
    
    virtual void    setPoint(const Coord& loc, const float val);
    uiString        infoMsg() const	{ return infomsg_; }
//    virtual bool	fillPar(IOPar&) const;
//    virtual bool	usePar(const IOPar&);
    
    virtual void    setTrcKeySampling(const TrcKeySampling&);
    virtual void    setTrcKeySampling(const ODPolygon<double>& polyROI); // real world polygon coords 
    virtual void    setTrcKeySampling(); // based on data extents and survey
    
//    static const char*	sKeyNrFaults();
//    static const char*	sKeyFaultID();
    
    
    
    virtual bool grid( TaskRunner* tr=0 );
//    void addPolyMask( ODPolygon poly, bool outside=true );
//    void getConvexHull( ODPolygon& poly );
//    void getConcaveHull( ODPolygon& poly );
//    void maskGrid();
    
    static const char*  sKeyRowStep();
    static const char*  sKeyColStep();
    static const char*  sKeyMethod();
    static const char*  sKeyPolyNode(); 
    static const char*  sKeySearchRadius(); 
    static const char*  sKeyScopeType();
    static const char*  sKeyFaultID();
    static const char*  sKeyFaultNr();
    
protected:
    
    wmGridder2D();
    
    std::vector<float>      vals_;
    std::vector<TPoint>     locs_; 
    kdbush::KDBush<TPoint, std::size_t>  kdtree_;
    Interval<float>         inlrg_;
    Interval<float>         crlrg_;
    float                   searchradius_;
    
    Array2DImpl<float>      grid_;
    TrcKeySampling          hs_;
    
    uiString                infomsg_;
    
    virtual void        interpolate_( const int idx, const int idy ) {}
    
};


class wmIDWGridder2D : public wmGridder2D
{ mODTextTranslationClass(wmIDWGridder2D);
public:
    mDefaultFactoryInstantiation( wmGridder2D, wmIDWGridder2D, "InverseDistance", tr("Inverse distance") );
    
    wmIDWGridder2D();
    
    void            setPower(float);
    float           getPower() const { return pow_; }
    
    static const char* sKeyPower();
    
    
protected:
    float   pow_;
    
    void    interpolate_( const int idx, const int idy );
    
};
#endif
