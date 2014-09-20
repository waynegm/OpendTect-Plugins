/*Copyright (C) 2014 Wayne Mogg All rights reserved.

This file may be used either under the terms of:

1. The GNU General Public License version 3 or higher, as published by
the Free Software Foundation, or

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef mymath_h
#define mymath_h

/*+
________________________________________________________________________

 Author:        Wayne Mogg
 Date:          August 2014
 ________________________________________________________________________

-*/ 
namespace myMath
{
	int factorial(const int t)
	{ return ( t == 0 ) ? 1 : t * myMath::factorial( t - 1 ); } 
}
#endif