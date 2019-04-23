/*Copyright (C) 2019 Wayne Mogg All rights reserved.
 T his f*ile may be used either under the terms of:
 1. The GNU General Public License version 3 or higher, as published by
 the Free Software Foundation, or
 This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef deriv_eigen_h
#define deriv_eigen_h

/*+
 _ _____*__________________________________________________________________
 Author:        Wayne Mogg
 Date:          April 2019
 ________________________________________________________________________
 -*/ 
#include "Eigen/Core"

namespace derivEigen{

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

}
#endif 
