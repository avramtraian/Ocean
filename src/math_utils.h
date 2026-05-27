// Copyright (c) 2026 Traian Avram. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause.

#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include "core.h"

// For functions like square root, trigonometric, logarithm, etc.
// Providing our own implementation of these functions would most
// likely not produce any faster routines... -- avrtraian 27 May 2026
#include <math.h>

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
Clamp(T Value, T MinBound, T MaxBound)
{
    T Result = Min(MaxBound, Max(MinBound, Value));
    return Result;
}

template<typename T> inline T
Abs(T Value)
{
    T Result = (Value < cast(T, 0)) ? -Value : Value;
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

template<typename T, typename Q> inline T
Lerp(T ValueA, T ValueB, Q Factor)
{
    T Result = ValueA + (ValueB - ValueA) * Factor;
    return Result;
}

template<typename Q, typename T> inline Q
InvLerp(T ValueA, T ValueB, T Value)
{
    Q Result = cast(Q, Value - ValueA) / cast(Q, ValueB - ValueA);
    return Result;
}

template<typename T, typename Q> inline Q
LinearMap(T SrcValueA, T SrcValueB, T SrcValue, Q DstValueA, Q DstValueB)
{
    f32 Factor = InvLerp<f32>(SrcValueA, SrcValueB, SrcValue);
    Q Result = Lerp(DstValueA, DstValueB, Factor);
    return Result;
}

inline f32
Sqrt(f32 Value)
{
    f32 Result = sqrtf(Value);
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

inline v2f
V2f(f32 X, f32 Y)
{
    v2f Result;
    Result.X = X;
    Result.Y = Y;
    return Result;
}

inline v2f
V2f(v2u Other)
{
    v2f Result;
    Result.X = cast(f32, Other.X);
    Result.Y = cast(f32, Other.Y);
    return Result;
}

inline f32
Dot(v2f A, v2f B)
{
    f32 Result = (A.X * B.X) + (A.Y * B.Y);
    return Result;
}

inline v2f
operator+(v2f LHS, v2f RHS)
{
    v2f Result;
    Result.X = LHS.X + RHS.X;
    Result.Y = LHS.Y + RHS.Y;
    return Result;
}

inline v2f
operator*(v2f LHS, f32 RHS)
{
    v2f Result;
    Result.X = LHS.X * RHS;
    Result.Y = LHS.Y * RHS;
    return Result;
}

inline v2f
operator*(f32 LHS, v2f RHS)
{
    v2f Result;
    Result.X = LHS * RHS.X;
    Result.Y = LHS * RHS.Y;
    return Result;
}

inline v2s
V2s(s32 X, s32 Y)
{
    v2s Result;
    Result.X = X;
    Result.Y = Y;
    return Result;
}

inline v2u
V2u(u32 X, u32 Y)
{
    v2u Result;
    Result.X = X;
    Result.Y = Y;
    return Result;
}

struct rect2s
{
    v2s MinPoint;
    v2s MaxPoint;
};

inline rect2s
Rect2s(s32 MinX, s32 MinY, s32 MaxX, s32 MaxY)
{
    rect2s Result;
    Result.MinPoint = V2s(MinX, MinY);
    Result.MaxPoint = V2s(MaxX, MaxY);
    return Result;
}

inline b8
IsDegenerated(rect2s Rect)
{
    b8 Result = Rect.MinPoint.X >= Rect.MaxPoint.X ||
                Rect.MinPoint.Y >= Rect.MaxPoint.Y;
    return Result;
}

inline rect2s
Intersect(rect2s A, rect2s B)
{
    rect2s Result;
    Result.MinPoint.X = Max(A.MinPoint.X, B.MinPoint.X);
    Result.MinPoint.Y = Max(A.MinPoint.Y, B.MinPoint.Y);
    Result.MaxPoint.X = Min(A.MaxPoint.X, B.MaxPoint.X);
    Result.MaxPoint.Y = Min(A.MaxPoint.Y, B.MaxPoint.Y);
    return Result;
}

inline u32
GetWidth(rect2s Rect)
{
    u32 Result = Rect.MaxPoint.X - Rect.MinPoint.X;
    return Result;
}

inline u32
GetHeight(rect2s Rect)
{
    u32 Result = Rect.MaxPoint.Y - Rect.MinPoint.Y;
    return Result;
}

inline v2u
GetSize(rect2s Rect)
{
    v2u Result;
    Result.X = GetWidth(Rect);
    Result.Y = GetHeight(Rect);
    return Result;
}

struct linear_color
{
    u8 B;
    u8 G;
    u8 R;
    u8 A;
};

inline linear_color
LinearColor(u8 R, u8 G, u8 B)
{
    linear_color Result;
    Result.R = R;
    Result.G = G;
    Result.B = B;
    Result.A = 0xFF;
    return Result;
}

inline u32
PackLinearToBGRA(linear_color Color)
{
    // The 'linear_color' structure is specifically declared to
    // ensure this is correct (and fast!) -- avrtraian 24 May 2026
    u32 Packed = *cast(u32*, &Color);
    return Packed;
}

inline linear_color
UnpackLinearFromBGRA(u32 Color)
{
    // The 'linear_color' structure is specifically declared to
    // ensure this is correct (and fast!) -- avrtraian 24 May 2026
    linear_color Unpacked = *cast(linear_color*, &Color);
    return Unpacked;
}

#endif // MATH_UTILS_H
