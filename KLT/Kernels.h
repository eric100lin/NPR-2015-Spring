/*

 Copyright (C) 2004, Aseem Agarwala, roto@agarwala.org

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or (at
 your option) any later version.

 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 USA

 */

#ifndef KERNELS_H
#define KERNELS_H

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include "Error.h"

#define MAX_KERNEL_WIDTH 	151

struct ConvolutionKernel
{
    int width;
    float data[MAX_KERNEL_WIDTH];
};

class Kernels
{

public:

    Kernels(const float sigma);

    float getSigma() const
    {
        return _sigma;
    }

    const ConvolutionKernel* gauss() const
    {
        return &_gauss;
    }
    const ConvolutionKernel* gaussDeriv() const
    {
        return &_gauss_deriv;
    }

private:

    float _sigma;
    ConvolutionKernel _gauss, _gauss_deriv;
};

#endif
