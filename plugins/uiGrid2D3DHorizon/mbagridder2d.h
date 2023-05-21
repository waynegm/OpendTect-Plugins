#ifndef mbagridder2d_h
#define mbagridder2d_h

#include "wmgridder2d.h"
class uiParent;

class wmMBAGridder2D : public wmGridder2D
{
public:
    wmMBAGridder2D();
    ~wmMBAGridder2D() {}

    bool		prepareForGridding(uiParent*);
    bool		executeGridding(uiParent*);

protected:

    int				maxlevels_;

};

#endif

