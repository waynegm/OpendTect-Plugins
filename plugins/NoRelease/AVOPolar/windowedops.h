/*Copyright (C) 2018 Wayne Mogg All rights reserved.
 T his f*ile may be used either under the terms of:
 1. The GNU General Public License version 3 or higher, as published by
 the Free Software Foundation, or
 This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef windowedops_h
#define windowedops_h

/*+
 _ _____*__________________________________________________________________
 Author:        Wayne Mogg
 Date:          December 2018
 ________________________________________________________________________
 -*/ 
#include "arrayndimpl.h"

namespace windowedOps{
    
template <class T>
void sum( const Array1DImpl<T>& input, const int winSize, Array1DImpl<T>& output )
{
    int sz = input.info().getSize(0);
    output.setSize(sz);
    int halfWinSize = (winSize-1)/2;
    
    for (int idx=0; idx<sz; idx++) {
        int beg = idx-halfWinSize;
        beg = beg<0 ? 0: beg;
        int end = beg + winSize;
        end = end>sz ? sz: end;
        double tmp = 0.0;
        for (int i=beg; i<end; i++)
            tmp += input.get(i);
        output.set(idx, tmp);
    }
} 
    
template <class T>    
void min( const Array1DImpl<T>& input, const int winSize, Array1DImpl<T>& output )
{
    int sz = input.info().getSize(0);
    output.setSize(sz);
    int halfWinSize = (winSize-1)/2;
    
    for (int idx=0; idx<sz; idx++) {
        int beg = idx-halfWinSize;
        beg = beg<0 ? 0: beg;
        int end = beg + winSize;
        end = end>sz ? sz: end;
        T tmp = input.get(idx);
        for (int i=beg; i<end; i++) {
            T val = input.get(i);
            tmp = val<tmp ? val: tmp;
        }
        output.set(idx, tmp);
    }
} 

template <class T>    
void max( const Array1DImpl<T>& input, const int winSize, Array1DImpl<T>& output )
{
    int sz = input.info().getSize(0);
    output.setSize(sz);
    int halfWinSize = (winSize-1)/2;
    
    for (int idx=0; idx<sz; idx++) {
        int beg = idx-halfWinSize;
        beg = beg<0 ? 0: beg;
        int end = beg + winSize;
        end = end>sz ? sz: end;
        T tmp = input.get(idx);
        for (int i=beg; i<end; i++) {
            T val = input.get(i);
            tmp = val>tmp ? val: tmp;
        }
        output.set(idx, tmp);
    }
} 

void minOld( const Array1DImpl<float>& input, const int winSize, Array1DImpl<float>& output )
{
    int sz = input.info().getSize(0);
    output.setSize(sz);
    
    if (sz<winSize) {
        float tmp = input.get(0);
        for (int idx=0; idx<sz; idx++)
            tmp = tmp<=input.get(idx) ? tmp: input.get(idx);
        output.setAll(tmp);
    } else {
        float tmp = input.get(0);
        int halfWinSize = (winSize-1)/2;
        for (int idx=0; idx<=halfWinSize; idx++)
            tmp = tmp<=input.get(idx) ? tmp: input.get(idx);
        output.set(0, tmp);
        
        for (int idx=1; idx<sz; idx++) {
            int isub = idx - halfWinSize -1;
            isub = isub<0? 0: isub;
            int iadd = idx + halfWinSize;
            iadd = iadd<sz ? iadd: sz-1;  
            if (tmp == input.get(isub)) {
                tmp = input.get(iadd);
                for (int i=iadd; i>isub; i--)
                    tmp = tmp<=input.get(i) ? tmp: input.get(i);
            }
            output.set( idx, tmp);
        }
    }
} 

    void maxOld( const Array1DImpl<float>& input, const int winSize, Array1DImpl<float>& output )
    {
        int sz = input.info().getSize(0);
        output.setSize(sz);
        
        if (sz<winSize) {
            float tmp = input.get(0);
            for (int idx=0; idx<sz; idx++)
                tmp = tmp>=input.get(idx) ? tmp: input.get(idx);
            output.setAll(tmp);
        } else {
            float tmp = input.get(0);
            int halfWinSize = (winSize-1)/2;
            for (int idx=0; idx<=halfWinSize; idx++)
                tmp = tmp>=input.get(idx) ? tmp: input.get(idx);
            output.set(0, tmp);
            
            for (int idx=1; idx<sz; idx++) {
                int isub = idx - halfWinSize -1;
                isub = isub<0? 0: isub;
                int iadd = idx + halfWinSize;
                iadd = iadd<sz ? iadd: sz-1;  
                if (tmp == input.get(isub)) {
                    tmp = input.get(iadd);
                    for (int i=iadd; i>isub; i--)
                        tmp = tmp>=input.get(i) ? tmp: input.get(i);
                }
                output.set( idx, tmp);
            }
        }
    } 
    
    void minIdx( const Array1DImpl<float>& input, const int winSize, Array1DImpl<int>& output )
    {
        int sz = input.info().getSize(0);
        output.setSize(sz);
        
        if (sz<winSize) {
            int tmp = 0;
            for (int idx=0; idx<sz; idx++)
                tmp = input.get(tmp)<=input.get(idx) ? tmp: idx;
            output.setAll(tmp);
        } else {
            int tmp = 0;
            int halfWinSize = (winSize-1)/2;
            for (int idx=0; idx<=halfWinSize; idx++)
                tmp = input.get(tmp)<=input.get(idx) ? tmp: idx;
            output.set(0, tmp);
            
            for (int idx=1; idx<sz; idx++) {
                int isub = idx - halfWinSize -1;
                isub = isub<0? 0: isub;
                int iadd = idx + halfWinSize;
                iadd = iadd<sz ? iadd: sz-1;  
                if (tmp == isub) {
                    tmp = iadd;
                    for (int i=iadd; i>isub; i--)
                        tmp = input.get(tmp)<=input.get(i) ? tmp: i;
                }
                output.set( idx, tmp);
            }
        }
    } 
    
    void maxIdx( const Array1DImpl<float>& input, const int winSize, Array1DImpl<int>& output )
    {
        int sz = input.info().getSize(0);
        output.setSize(sz);
        
        if (sz<winSize) {
            int tmp = input.get(0);
            for (int idx=0; idx<sz; idx++)
                tmp = input.get(tmp)>=input.get(idx) ? tmp: idx;
            output.setAll(tmp);
        } else {
            int tmp = input.get(0);
            int halfWinSize = (winSize-1)/2;
            for (int idx=0; idx<=halfWinSize; idx++)
                tmp = input.get(tmp)>=input.get(idx) ? tmp: idx;
            output.set(0, tmp);
            
            for (int idx=1; idx<sz; idx++) {
                int isub = idx - halfWinSize -1;
                isub = isub<0? 0: isub;
                int iadd = idx + halfWinSize;
                iadd = iadd<sz ? iadd: sz-1;  
                if (tmp == isub) {
                    tmp = iadd;
                    for (int i=iadd; i>isub; i--)
                        tmp = input.get(tmp)>=input.get(i) ? tmp: i;
                }
                output.set( idx, tmp);
            }
        }
    } 
    
}
#endif 
