#ifndef rbfgridder_h
#define rbfgridder_h

#include "wmgridder2d.h"



class wmRBFGridder2D : public wmGridder2D
{
public:
    wmRBFGridder2D();
    ~wmRBFGridder2D() {}
    
    bool	executeGridding(TaskRunner*);
    bool    	usePar(const IOPar&);
protected:
    float	tension_;
};

#endif
