// Copyright (c) 2026 Traian Avram. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause.

#ifndef MATH_H
#define MATH_H

#include "core.h"

template<typename T> inline T
Min(T A, T B)
{
    T Result = (A < B) ? A : B;
    return Result;
}

template<typename T> inline T
Max(T A, T B)
{
    T Result = (A > B) ? A : B;
    return Result;
}

template<typename T> inline T
RoundDownPowOfTwo(T Value, T PowOfTwo)
{
    T Mask = PowOfTwo - 1;
    T Result = Value & (~Mask);
    return Result;
}

template<typename T> inline T
RoundUpPowOfTwo(T Value, T PowOfTwo)
{
    T Mask = PowOfTwo - 1;
    T Result = (Value + Mask) & (~Mask);
    return Result;
}

struct v2f
{
    f32 X;
    f32 Y;
};

struct v2s
{
    s32 X;
    s32 Y;
};

struct v2u
{
    u32 X;
    u32 Y;
};

inline v2u
V2u(u32 X, u32 Y)
{
    v2u Result;
    Result.X = X;
    Result.Y = Y;
    return Result;
}

#endif // MATH_H
