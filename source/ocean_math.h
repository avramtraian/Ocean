/*  =====================================================================
    $File:   ocean_math.h $
    $Date:   September 19 2023 $
    $Author: Traian Avram $
    $Notice: Copyright (c) 2023-2023 Traian Avram. All Rights Reserved. $
    =====================================================================  */
#ifndef OCEAN_MATH_H

struct v2s
{
    s32 X;
    s32 Y;
};

struct extent2
{
    s32 Width;
    s32 Height;
};

struct offset2
{
    s32 X;
    s32 Y;
};

struct rectangle2
{
    offset2 Offset;
    extent2 Extent;
};

inline v2s
operator+(v2s A, v2s B)
{
    v2s Result;
    Result.X = A.X + B.X;
    Result.Y = A.Y + B.Y;
    return Result;
}

inline v2s
operator-(v2s A, v2s B)
{
    v2s Result;
    Result.X = A.X - B.X;
    Result.Y = A.Y - B.Y;
    return Result;
}

internal inline u32
PackRGBA(u8 R, u8 G, u8 B, u8 A = 0xFF)
{
    u32 Packed = 0;
    Packed |= ((u32)A << 24);
    Packed |= ((u32)R << 16);
    Packed |= ((u32)G <<  8);
    Packed |= ((u32)B <<  0);
    return Packed;
}

internal inline u32
PackRGBA(u8 Scalar)
{
    u32 Result = PackRGBA(Scalar, Scalar, Scalar, 0xFF);
    return Result;
}

internal inline void
UnpackRGBA(u32 Packed, u8 *R, u8 *G, u8 *B, u8 *A = NULL)
{
    if (A) { *A = ((Packed >> 24) & 0xFF); }
    if (R) { *R = ((Packed >> 16) & 0xFF); }
    if (G) { *G = ((Packed >>  8) & 0xFF); }
    if (B) { *B = ((Packed >>  0) & 0xFF); }
}

#define OCEAN_MATH_H
#endif // OCEAN_MATH_H
