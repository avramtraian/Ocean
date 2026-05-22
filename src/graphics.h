// Copyright (c) 2026 Traian Avram. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause.

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "core.h"

typedef u8 bitmap_format;
enum bitmap_format_enum : bitmap_format
{
    BitmapFormat_Unknown = 0,
    BitmapFormat_BGRA_8888,
};

struct bitmap
{
    u8*           Data;
    u32           Width;
    u32           Height;
    bitmap_format Format;
};

#endif // GRAPHICS_H
