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
typedef kdbush::KDBush<TPoint, std::size_t> KDTree;

class wmGridder2D
{ mODTextTranslationClass(wmGridder2D);
public:
    friend class LocalInterpolator;
    enum ScopeType   { BoundingBox, Horizon };
    enum Method { LocalRBF, MBA, IDW }; 
    static const char*	ScopeNames[];
    static const char*	MethodNames[];
    static wmGridder2D*	create(const char* methodName);
    static bool		canHandleFaultPolygons(const char* methodName);
    
    virtual		~wmGridder2D();

    void		setBlockSize(float bs) { blocksize_ = bs; }
    float		getBlockSize() const { return blocksize_; }

    void		setPercOverlap(int polp) { percoverlap_ = polp; }
    int			getPercOverlap() const { return percoverlap_; }
    
    virtual void	setPoint(const Coord& binLoc, const float val);
    void		includeInRange(const Coord& binLoc);
    
    virtual bool	prepareForGridding();
    virtual bool	executeGridding(TaskRunner*) = 0;
    
    virtual bool	usePar(const IOPar&);
    
    TrcKeySampling	getTrcKeySampling() const;
    bool		loadData();
    bool		setScope();
    bool		isUncropped( Coord ) const;
    bool		inFaultHeave( Coord ) const;
    static bool		segmentsIntersect(Coord, Coord, Coord, Coord);
    bool		faultBetween( Coord, Coord) const;
    void		getHorRange(Interval<int>&, Interval<int>&);
    bool		saveGridTo(EM::Horizon3D*);
    
    static const char*  sKeyScopeHorID();
    static const char*  sKeyRowStep();
    static const char*  sKeyColStep();
    static const char*  sKeyMethod();
    static const char*  sKeyClipPolyID(); 
    static const char*  sKeyBlockSize(); 
    static const char*  sKeyOverlap(); 
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
    std::vector<TPoint>     binLocs_; 
    KDTree		    kdtree_;
    Interval<float>         inlrg_;
    Interval<float>         crlrg_;
    float                   blocksize_;
    int		    	    percoverlap_;
    
    Array2DImpl<float>*     grid_;
    Array2DImpl<float>*     carr_;
    TypeSet<od_int64>	    interpidx_;
    TrcKeySampling          hs_;
    
    const TaskRunner*	    tr_;
    
    void                    localInterp();
};

#endif
