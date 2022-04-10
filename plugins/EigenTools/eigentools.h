#pragma once

/*
 *   Eigen Miscellaneous Functions
 *   Copyright (C) 2021  Wayne Mogg
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
#include "commondefs.h"

namespace EigenTools{

template <typename X>
void cosTaper(int width, Eigen::DenseBase<X>& data)
{
    if (data.size()<2*width)
	return;

    X taper = X::LinSpaced(width, -M_PI, 0.f);
    taper << (taper.array().cos()+1)/2;
    data.head(width) = data.head(width).array() * taper.array();
    data.tail(width) = data.tail(width).array() * taper.reverse().array();
}

template <typename X, typename Y>
void mirrorPad(const Eigen::DenseBase<X>& input, Eigen::DenseBase<Y>& output)
{
    const int ns = input.size();
    const int pivot = ns%2==0 ? ns/2 : (ns+1)/2;
    output << input.head(ns-pivot).reverse(), input, input.tail(pivot).reverse();
}

template <typename X, typename Y>
void symmetricPad(const Eigen::DenseBase<X>& input, Eigen::DenseBase<Y>& output)
{
    const int ns = input.size();
    const int pivot = ns%2==0 ? ns/2 : (ns+1)/2;
    X trinput = input.segment(1,ns-2);
    output << trinput.head(ns-pivot).reverse(), input, trinput.tail(pivot).reverse();
}

template <typename X, typename Y>
void fftshift(const Eigen::DenseBase<X>& input, Eigen::DenseBase<Y>& output)
{
    const int ns = input.size();
    const int pivot = ns%2==0 ? ns/2 : (ns+1)/2;
    output << input.tail(ns-pivot), input.head(pivot);
}

}
