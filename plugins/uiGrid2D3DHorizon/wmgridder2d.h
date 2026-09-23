#pragma once

#include <cstddef>
#include "factory.h"
#include "trckeysampling.h"
#include "polygon.h"
#include "arrayndimpl.h"
#include "multiid.h"
#include "paralleltask.h"

class TaskRunner;
class TrcKeySampling;
class LocalInterpolator;
class uiParent;
namespace EM { class Horizon3D; }


class wmGridder2D
{ mODTextTranslationClass(wmGridder2D);
public:
    friend class LocalInterpolator;
    friend class GridMask;
    enum ScopeType   { Range, BoundingBox, ConvexHull, Horizon };
    enum Method { LTPS, MBA, IDW, NRN, ITER };
    static const char*	ScopeNames[];
    static const char*	MethodNames[];
    static wmGridder2D*	create(const char* methodName);
    static bool		canHandleFaultPolygons(const char* methodName);

    virtual		~wmGridder2D();

    virtual void	setPoint(const Coord& binLoc, const float val);
    void		includeInRange(const Coord& binLoc);

    virtual bool	prepareForGridding(uiParent*);
    virtual bool	executeGridding(uiParent*) = 0;

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

    static const char*	sKeyInput();
    static const char*	sKeyGridDef();
    static const char*  sKeyScopeHorID();
    static const char*  sKeyMethod();
    static const char*  sKeyClipPolyID();
    static const char*  sKeySearchRadius();
    static const char*  sKeyMaxPoints();
    static const char*  sKeyScopeType();
    static const char*  sKeyFaultPolyID();
    static const char*  sKeyFaultPolyNr();
    static const char*  sKeyFaultID();
    static const char*  sKeyFaultNr();
    static const char*  sKeyContourPolyID();
    static const char*  sKeyContourPolyNr();
    static const char*  sKey2DHorizonID();
    static const char*  sKey2DLineIDNr();
    static const char*  sKey2DLineID();
    static const char*  sKey3DHorizonID();
    static const char*  sKeyRegularization();
    static const char*  sKeyTension();
    static const char*  sKeySmoothing();
    static const char*  sKeySmoothingRadius();

protected:

    wmGridder2D();

    MultiID                 hor2DID_;
    TypeSet<Pos::GeomID>    geomids_;
    MultiID                 hor3DID_;
    TrcKeySampling          hor3Dsubsel_;

    MultiID					croppolyID_;
    ODPolygon<Pos::Ordinate_Type>		croppoly_;

    TypeSet<MultiID>				contpolyID_;

    TypeSet<MultiID>				faultpolyID_;
    ObjectSet<ODPolygon<Pos::Ordinate_Type>>	faultpoly_;

    TypeSet<MultiID>				faultids_;

    ScopeType					scope_;
    MultiID					horScopeID_;
    ODPolygon<Pos::Ordinate_Type>		cvxhullpoly_;

    float					searchradius_;
    int						maxpoints_;

    TypeSet<float>				vals_;
    TypeSet<Coord>				binLocs_;
    Interval<Pos::Ordinate_Type>		inlrg_;
    Interval<Pos::Ordinate_Type>		crlrg_;

    Array2DImpl<float>*				grid_ = nullptr;
    Array2DImpl<float>*				carr_ = nullptr;
    TypeSet<od_int64>				interpidx_;
    TrcKeySampling				hs_;

    bool	localInterp(uiParent*, bool approximation = true);
    void	logGriddedDataQC(od_int64 ndata, double datasum,
				 float datamin, const BinID& dataminbid,
				 float datamax, const BinID& datamaxbidx) const;
    void	reportMemError(const char*, const char*, od_int64);

    bool	smoothGrid(uiParent*);

    class GridSmoother;
    class SmoothPasses;

    Array2DImpl<unsigned char>*		fixedmask_ = nullptr;
    int					smoothpasses_ = 0;
    int					smoothradius_ = 2;
};

