#pragma once

/*
 *   Eigen Filter functions
 *   Copyright (C) 2019  Wayne Mogg
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "eigen3/Eigen/Core"

namespace EigenFilter{
/*
 * Apply single pass recursive digital filter, arrays assumed to be 1D, works for complex filters
 */
template <typename X>
void iirfilt(const Eigen::ArrayBase<X>& input, const Eigen::ArrayBase<X>& num, const Eigen::ArrayBase<X>& den, Eigen::ArrayBase<X>& output)
{
    const int ns = input.size();
    const int nn = num.size();
    const int nd = den.size();
    eigen_assert(ns == output.size());

    X inpad(ns+nn-1);
    inpad.setZero();
    inpad.tail(ns) = input;
//    Eigen::ArrayBase<X>& outRef = const_cast<Eigen::ArrayBase<X>&>(output);
    output.setZero();
    for (int i=0; i<nn; i++) {
        output += num[i]*inpad.segment(nn-i-1,ns);
    }
    X revden = den.reverse();
    for (int i=0; i<nd; i++) {
        for (int j=0; j<nd; j++) {
            if (i-j-1 >= 0)
                output[i] -= den[j]*output[i-j-1];
        }
    }
    for (int i=nd; i<ns; i++){
        output[i] -= (revden * output.segment(i-nd,nd)).sum();
    }
}

/*
 * Apply forward and reverse pass of recursive digital filter, arrays assumed to be 1D, non-complex datatype
 */
template <typename X>
void iirfiltfilt(const Eigen::ArrayBase<X>& input, const Eigen::ArrayBase<X>& num, const Eigen::ArrayBase<X>& den, Eigen::ArrayBase<X> const & output)
{
        X work(input.size());
        iirfilt(input, num, den, work);
        work.reverseInPlace();
        Eigen::ArrayBase<X>& outRef = const_cast<Eigen::ArrayBase<X>&>(output);
        iirfilt(work, num, den, outRef);
        outRef.reverseInPlace();
}

/*
 * Apply forward and reverse pass of recursive digital filter, arrays assumed to be 1D, for complex datatype
 */
void iirfiltfilt(const Eigen::ArrayXcd& input, const Eigen::ArrayXcd& num, const Eigen::ArrayXcd& den, Eigen::ArrayXcd const & output)
{
    Eigen::ArrayXcd work(input.size());
    iirfilt(input, num, den, work);
    work.reverseInPlace();
    Eigen::ArrayXcd cnum = conj(num);
    Eigen::ArrayXcd cden = conj(den);
    Eigen::ArrayXcd& outRef = const_cast<Eigen::ArrayXcd&>(output);
    iirfilt(work, cnum, cden, outRef);
    outRef.reverseInPlace();
}
}
