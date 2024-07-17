/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#pragma once

#include <Core/Types.h>

typedef struct LinearColor {
    // NOTE: The layout of the LinearColor structure is specifically designed to allow for
    //       trivial conversion between packed and unpacked. DO NOT CHANGE IT!
    u8 blue;
    u8 green;
    u8 red;
    u8 alpha;
} LinearColor;

inline LinearColor linear_color(u8 red, u8 green, u8 blue, u8 alpha = 255)
{
    LinearColor color;
    color.red = red;
    color.green = green;
    color.blue = blue;
    color.alpha = alpha;
    return color;
}

inline u32 color_pack_linear_to_u32_bgra(LinearColor color)
{
    // NOTE: The layout of the LinearColor structure is specifically designed to allow for
    //       trivial conversion between packed and unpacked.
    const u32* packed = (const u32*)(&color);
    return *packed;
}

inline LinearColor color_unpack_linear_from_u32_bgra(u32 packed)
{
    // NOTE: The layout of the LinearColor structure is specifically designed to allow for
    //       trivial conversion between packed and unpacked.
    const LinearColor* linear = (const LinearColor*)(&packed);
    return *linear;
}
