/*Copyright (C) 2019 Wayne Mogg All rights reserved.
 This file may be used either under the terms of:
 1. The GNU General Public License version 3 or higher, as published by
 the Free Software Foundation, or
 This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef phaseangle_h
#define phaseangle_h

/*+
 ___________________________________________________________________________
 Author:        Wayne Mogg
 Date:          June 2019
 ________________________________________________________________________
 -*/  

#include <complex>

namespace WMLib
{
class PhaseAngle
{
public:
    static PhaseAngle radians( double r ) { return PhaseAngle(r); }
    static PhaseAngle degrees( double d ) { return PhaseAngle(d/180.0*M_PI); }
    static PhaseAngle mean( const PhaseAngle& a, const PhaseAngle& b ) { return PhaseAngle(a.val_+b.val_); }
    
    double radians() const { return std::arg(val_); }
    double degrees() const { return radians()/M_PI*180.0; }
    
    PhaseAngle& operator+=( const PhaseAngle& o) { val_ *= o.val_; return *this; }
    PhaseAngle& operator-=( const PhaseAngle& o) { val_ *= std::conj(o.val_); return *this; }
 
protected:
    std::complex<double> val_;
    
    PhaseAngle( double r ) : val_(std::complex<double>(cos(r), sin(r))) {}
    PhaseAngle( std::complex<double> c ) : val_(c/std::abs(c)) {}
    
};

inline PhaseAngle operator+(const PhaseAngle& x, const PhaseAngle& y )
{
    PhaseAngle res = x;
    res += y;
    return res;
}
    
inline PhaseAngle operator-(const PhaseAngle& x, const PhaseAngle& y )
{
    PhaseAngle res = x;
    res -= y;
    return res;
}
    
}; //namespace

#endif
