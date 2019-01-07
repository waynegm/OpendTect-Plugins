/*Copyright (C) 2018 Wayne Mogg All rights reserved.
 T his f*ile may be used either under the terms of:
 1. The GNU General Public License version 3 or higher, as published by
 the Free Software Foundation, or
 This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef windowedops_eigen_h
#define windowedops_eigen_h

/*+
 _ _____*__________________________________________________________________
 Author:        Wayne Mogg
 Date:          December 2018
 ________________________________________________________________________
 -*/ 
#include "Eigen/Core"

namespace windowedOpsEigen{
    
void sum( const Eigen::ArrayXd& input, const int winSize, Eigen::ArrayXd& output )
{
    int sz = input.size();
    output.resize(sz);
    int halfWinSize = (winSize-1)/2;
    
    for (int idx=0; idx<sz; idx++) {
        int beg = idx-halfWinSize;
        beg = beg<0 ? 0: beg;
        int n = winSize;
        n = beg+n>=sz ? sz-beg : n;
        output(idx) = input.segment(beg,n).sum();
    }
} 

}
#endif 
