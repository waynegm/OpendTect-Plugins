#ifndef mbagridder2d_h
#define mbagridder2d_h

#include "mba.hpp"
#include "wmgridder2d.h"

class wmMBAGridder2D : public wmGridder2D
{
public:
    wmMBAGridder2D();
    ~wmMBAGridder2D() {}
    
    bool		prepareForGridding();
    bool		executeGridding(TaskRunner*);
    
protected:
    
    int				maxlevels_;
    
};

#endif

