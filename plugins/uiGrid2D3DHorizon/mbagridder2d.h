#ifndef mbagridder2d_h
#define mbagridder2d_h

#include "mba.hpp"
#include "wmgridder2d.h"

class wmMBAGridder2D : public wmGridder2D
{
public:
    wmMBAGridder2D();
    ~wmMBAGridder2D() {}
    
    void		setPoint(const Coord& binLoc, const float val);
    bool		prepareForGridding();
    bool		executeGridding(TaskRunner*);
    
protected:
    std::vector<mba::point<2>> 	binLocs_;
    std::vector<float> 		vals_;
    
    int				maxlevels_;
    
};

#endif

