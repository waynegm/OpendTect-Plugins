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
        int end = idx+halfWinSize;
        beg = beg<0 ? 0: beg;
        end = end>=sz ? sz-1 : end;
        int n = end-beg+1;
        output(idx) = input.segment(beg,n).sum();
    }
} 



void minIdx( const Eigen::ArrayXd& input, const int winSize, Eigen::ArrayXi& output )
{
    int sz = input.size();
    output.resize(sz);
    int halfWinSize = (winSize-1)/2;
    Eigen::Index ind;
    
    for (int idx=0; idx<sz; idx++) {
        int beg = idx-halfWinSize;
        int end = idx+halfWinSize;
        beg = beg<0 ? 0: beg;
        end = end>=sz ? sz-1 : end;
        int n = end-beg+1;
        input.segment(beg,n).minCoeff(&ind);
        output[idx] = ind+beg;
    }
}

void maxIdx( const Eigen::ArrayXd& input, const int winSize, Eigen::ArrayXi& output )
{
    int sz = input.size();
    output.resize(sz);
    int halfWinSize = (winSize-1)/2;
    
    for (int idx=0; idx<sz; idx++) {
        Eigen::Index imax;
        int beg = idx-halfWinSize;
        int end = idx+halfWinSize;
        beg = beg<0 ? 0: beg;
        end = end>=sz ? sz-1 : end;
        int n = end-beg+1;
        input.segment(beg,n).maxCoeff(&imax);
        output[idx] = imax+beg;
    }
}    

void min( const Eigen::ArrayXd& input, const int winSize, Eigen::ArrayXd& output )
{
    int sz = input.size();
    output.resize(sz);
    int halfWinSize = (winSize-1)/2;
    
    for (int idx=0; idx<sz; idx++) {
        int beg = idx-halfWinSize;
        int end = idx+halfWinSize;
        beg = beg<0 ? 0: beg;
        end = end>=sz ? sz-1 : end;
        int n = end-beg+1;
        output[idx] = input.segment(beg,n).minCoeff();
    }
}    

void min( const Eigen::ArrayXd& input, const int winSize, Eigen::ArrayXd& output, Eigen::ArrayXi& indices )
{
    int sz = input.size();
    output.resize(sz);
    indices.resize(sz);
    int halfWinSize = (winSize-1)/2;
    Eigen::Index ind;
    
    for (int idx=0; idx<sz; idx++) {
        int beg = idx-halfWinSize;
        int end = idx+halfWinSize;
        beg = beg<0 ? 0: beg;
        end = end>=sz ? sz-1 : end;
        int n = end-beg+1;
        output[idx] = input.segment(beg,n).minCoeff(&ind);
        indices[idx] = ind+beg;
    }
}    

void max( const Eigen::ArrayXd& input, const int winSize, Eigen::ArrayXd& output )
{
    int sz = input.size();
    output.resize(sz);
    int halfWinSize = (winSize-1)/2;
    
    for (int idx=0; idx<sz; idx++) {
        int beg = idx-halfWinSize;
        int end = idx+halfWinSize;
        beg = beg<0 ? 0: beg;
        end = end>=sz ? sz-1 : end;
        int n = end-beg+1;
        output[idx] = input.segment(beg,n).maxCoeff();
    }
}    

void max( const Eigen::ArrayXd& input, const int winSize, Eigen::ArrayXd& output, Eigen::ArrayXi& indices )
{
    int sz = input.size();
    output.resize(sz);
    indices.resize(sz);
    int halfWinSize = (winSize-1)/2;
    Eigen::Index ind;
    
    for (int idx=0; idx<sz; idx++) {
        int beg = idx-halfWinSize;
        int end = idx+halfWinSize;
        beg = beg<0 ? 0: beg;
        end = end>=sz ? sz-1 : end;
        int n = end-beg+1;
        output[idx] = input.segment(beg,n).maxCoeff(&ind);
        indices[idx] = ind+beg;
    }
}    

}
#endif 
