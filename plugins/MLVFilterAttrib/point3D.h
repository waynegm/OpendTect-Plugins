/*Copyright (C) 2014 Wayne Mogg All rights reserved.

This file may be used either under the terms of:

1. The GNU General Public License version 3 or higher, as published by
the Free Software Foundation, or

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef point3D_h
#define point3D_h

/*+
________________________________________________________________________

 Author:        Wayne Mogg
 Date:          January 2014
 ________________________________________________________________________

-*/ 
namespace wmGeom
{

template <class T>
class Point3D
{
  public:
				Point3D(T xx=0,T yy=0, T zz=0);

    template <class TT>
    Point3D<T>&			setFrom(const Point3D<TT>&);

    template <class TT>
    inline void			setXYZ(TT xx,TT yy, TT zz);
    inline void			setXYZ(T xx,T yy, T zz);
    inline Point3D<T>&		zero();
    inline Point3D<T>		operator-();

    inline bool			operator==(const Point3D<T>&) const;
    inline bool			operator!=(const Point3D<T>&) const;
    inline Point3D<T>&		operator+=(T dist);
    inline Point3D<T>&		operator*=(T factor);
    inline Point3D<T>&		operator/=(T den);
    inline Point3D<T>&		operator+=(const Point3D<T>&);
    inline Point3D<T>&		operator-=(const Point3D<T>&);
    inline Point3D<T>		operator+(const Point3D<T>&) const;
    inline Point3D<T>		operator-(const Point3D<T>&) const;
    inline Point3D<T>		operator*(const T factor) const;
    inline Point3D<T>		operator/(const T den) const;
    
    inline bool			isDefined() const;
    inline double		abs() const;
    inline T			sqAbs() const;
    inline double		distTo(const Point3D<T>&) const;
    inline T			sqDistTo(const Point3D<T>&) const;
    inline T			dot(const Point3D<T>&) const;

    static Point3D<T>		udf() { return Point3D<T>(mUdf(T),mUdf(T)); }
    
    T 				x;
    T 				y;
    T				z;
  
}; 

template <class T> inline
Point3D<T>::Point3D ( T xx , T yy , T zz )
    : x(xx), y(yy), z(zz)
{}

template <class T> template <class TT> inline
Point3D<T>& Point3D<T>::setFrom( const Point3D<TT>& a )
{ x=a.x; y=a.y; z=a.z; return *this;}

template <class T> inline
void Point3D<T>::setXYZ( T xx, T yy, T zz )
{ x = xx ; y = yy; z=zz; }  

template <class T> template <class TT> inline
void Point3D<T>::setXYZ( TT xx, TT yy, TT zz )
{ x = (T)xx; y = (T)yy; z = (T)zz; }

template <class T> inline
Point3D<T>& Point3D<T>::zero()
{ x = y = z = 0; return *this; }

template <class T> inline
Point3D<T> Point3D<T>::operator -()
{ return Point3D<T>( -x, -y, -z ); }

template <class T> inline
bool Point3D<T>::operator ==( const Point3D<T>& p ) const
{ return p.x == x && p.y == y && p.z == z; }


template <class T> inline
bool Point3D<T>::operator !=( const Point3D<T>& p ) const
{ return !(*this==p); }

template <class T> inline
Point3D<T>& Point3D<T>::operator+=( T dist )
{ x += dist; y += dist; z += dist; return *this; }


template <class T> inline
Point3D<T>& Point3D<T>::operator*=( T factor )
{ x *= factor; y *= factor; z *= factor; return *this; }


template <class T> inline
Point3D<T>& Point3D<T>::operator/=( T den )
{ x /= den; y /= den; z /= den; return *this; }


template <class T> inline
Point3D<T>& Point3D<T>::operator +=( const Point3D<T>& p )
{ x += p.x; y += p.y; z += p.z; return *this; }


template <class T> inline
Point3D<T>& Point3D<T>::operator -=( const Point3D<T>& p )
{ x -= p.x; y -= p.y; z -= p.z; return *this; }


template <class T> inline
Point3D<T> Point3D<T>::operator +( const Point3D<T>& p ) const
{ return Point3D<T>(x+p.x,y+p.y,z+p.z); }


template <class T> inline
Point3D<T> Point3D<T>::operator -( const Point3D<T>& p ) const
{ return Point3D<T>(x-p.x,y-p.y,z-p.z); }


template <class T> inline
Point3D<T> Point3D<T>::operator *( const T factor ) const
{ return Point3D<T>(factor*x,factor*y,factor*z); }


template <class T> inline
Point3D<T> Point3D<T>::operator /( const T den ) const
{ return Point3D<T>(x/den,y/den,z/den); }


template <class T> inline
bool Point3D<T>::isDefined() const
{ return !mIsUdf(x) && !mIsUdf(y) && !mIsUdf(z); }
    
    
template <class T> inline
double Point3D<T>::abs() const
{ return ::Math::Sqrt( (double)sqAbs() ); }


template <class T> inline
T Point3D<T>::sqAbs() const
{ return x*x + y*y + z*z; }


template <class T> inline
double Point3D<T>::distTo( const Point3D<T>& pt ) const
{ return ::Math::Sqrt( (double)sqDistTo(pt) ); }


template <class T> inline
T Point3D<T>::sqDistTo( const Point3D<T>& pt ) const
{
    const T xdiff = x-pt.x;
    const T ydiff = y-pt.y;
    const T zdiff = z-pt.z;
    return xdiff*xdiff + ydiff*ydiff + zdiff*zdiff;
}

template <class T> inline
T Point3D<T>::dot( const Point3D<T>& pt ) const
{
    return x*pt.x + y*pt.y + z*pt.z;
}

  
}
#endif