#pragma once

#include "idxpair.h"
#include "typeset.h"
#include "coord.h"
#include "sortedlist.h"

namespace wmUtil {
typedef Pos::Ordinate_Type OrdType;

void wmSimplify3DPolyline( const TypeSet<Coord3>& indata, SortedList<int>& residx, const OrdType dX, const OrdType dY, const OrdType dZ)
{
    residx += 0;
    residx += indata.size()-1;
    TypeSet<IdxPair> work;
    work += IdxPair(0, indata.size()-1);

    while (work.size()>0) {
        IdxPair p = work.pop();
        int start = p.first;
        int end = p.second;
        Coord3 seg = (indata[end]-indata[start]).normalize();
        Coord3 offmax(0.0,0.0,0.0);
        int idxmax(0);
        for (int idx=start+1; idx<end; idx++) {
            Coord3 pos = indata[idx]-indata[start];
            Coord3 dif = pos - (pos.dot(seg))*seg;
            dif.x = abs(dif.x);
            dif.y = abs(dif.y);
            dif.z = abs(dif.z);

            if (dif.x>=offmax.x || dif.y>=offmax.y || dif.z>=offmax.z) {
                offmax = dif;
                idxmax = idx;
            }
        }

        if (offmax.x>=dX || offmax.y>=dY || offmax.z>=dZ) {
            residx += idxmax;
            if (idxmax-start > 1)
                work.push(IdxPair(start, idxmax));
            if (end-idxmax > 1)
                work.push(IdxPair(idxmax, end));
        }
    }
}
}; //namespace
