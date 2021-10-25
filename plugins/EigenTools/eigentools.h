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

namespace EigenTools{

template <typename X>
void mirrorPad(const Eigen::ArrayBase<X>& input, Eigen::ArrayBase<X>& output)
{
    const int ns = input.size();
    const int pivot = ns%2==0 ? ns/2 : (ns+1)/2;
    output << input.head(ns-pivot).reverse(), input, input.tail(pivot).reverse();
}

template <typename X>
void symmetricPad(const Eigen::ArrayBase<X>& input, Eigen::ArrayBase<X>& output)
{
    const int ns = input.size();
    const int pivot = ns%2==0 ? ns/2 : (ns+1)/2;
    X trinput = input.segment(1,ns-2);
    output << trinput.head(ns-pivot).reverse(), input, trinput.tail(pivot).reverse();
}

template <typename X>
void fftshift(const Eigen::ArrayBase<X>& input, Eigen::ArrayBase<X>& output)
{
    const int ns = input.size();
    const int pivot = ns%2==0 ? ns/2 : (ns+1)/2;
    output << input.tail(ns-pivot), input.head(pivot);
}

}
