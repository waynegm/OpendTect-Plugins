#ifndef eigenfilt_h
#define eigenfilt_h

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

#include "Eigen/Core"

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
};
/*
namespace EigenDeriv{

// 3 point 1st derivative finite difference along rows
void dr_3( const Eigen::ArrayXXd& input, Eigen::ArrayXXd& output )
{
    output.resizeLike(input);
    int cols = input.cols();
    Eigen::ArrayXXd in_m1(input.rows(), cols());
    in_m1.leftCols(cols-1) = input.rightCols(cols-1);
    in_m1.col(col-1) = 0.0;
    Eigen::ArrayXXd in_p1(input.rows(), cols);
    in_p1.rightCols(cols-1) = input.leftCols(cols-1);
    in_p1.col(0) = 0.0;
    output = in_p1 - in_m1;
} 
    
// 3 point 1st derivative finite difference along cols
void dc_3( const Eigen::ArrayXXd& input, Eigen::ArrayXXd& output )
{
    output.resizeLike(input);
    int rows = input.rows();
    Eigen::ArrayXXd in_m1(rows, input.cols());
    in_m1.topRows(rows-1) = input.bottomRows(rows-1);
    in_m1.row(rows-1) = 0.0;
    Eigen::ArrayXXd in_p1(rows, input.cols());
    in_p1.bottomRows(rows-1) = input.topRows(rows-1);
    in_p1.row(0) = 0.0;
    output = in_p1 - in_m1;
}
    
// 3 point 2nd derivative finite difference along columns
void dcc_3( const Eigen::ArrayXXd& input, Eigen::ArrayXXd& output )
{
    output.resizeLike(input);
    int rows = input.rows();
    Eigen::ArrayXXd in_m1(rows, input.cols());
    in_m1.topRows(rows-1) = input.bottomRows(rows-1);
    in_m1.row(rows-1) = 0.0;
    Eigen::ArrayXXd in_p1(rows, input.cols());
    in_p1.bottomRows(rows-1) = input.topRows(rows-1);
    in_p1.row(0) = 0.0;
    output = -2.0*input + in_m1 + in_p1;
} 

// 3 point 2nd derivative finite difference along rows
void drr_3( const Eigen::ArrayXXd& input, Eigen::ArrayXXd& output )
{
    output.resizeLike(input);
    int cols = input.cols();
    Eigen::ArrayXXd in_m1(input.rows(), cols());
    in_m1.leftCols(cols-1) = input.rightCols(cols-1);
    in_m1.col(col-1) = 0.0;
    Eigen::ArrayXXd in_p1(input.rows(), cols);
    in_p1.rightCols(cols-1) = input.leftCols(cols-1);
    in_p1.col(0) = 0.0;
    output = -2.0*input + in_m1 + in_p1;
} 


// 3 point 2nd cross derivative finite difference
void drc_3( const Eigen::ArrayXXd& input, Eigen::ArrayXXd& output )
{
    Eigen::ArrayXXd tmp;
    derivEigen::dr_3(input, tmp);
    derivEigen::dc_3(tmp, output);
} 

}*/
#endif 
