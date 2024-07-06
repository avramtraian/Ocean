/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#pragma once

#include <Core/Forward.h>
#include <Core/Types.h>

enum BitmapFormatEnum {
    BITMAP_FORMAT_UNKNOWN = 0,
    BITMAP_FORMAT_R8G8B8A8,
};
typedef u8 BitmapFormat;

typedef struct Bitmap {
    u32 width;
    u32 height;
    ReadWriteBytes pixels;
    BitmapFormat format;
} Bitmap;

typedef struct BitmapClearColor {
    float red;
    float green;
    float blue;
    float alpha;
} BitmapClearColor;

// NOTE: The pixels are all black, as the memory returned by the arena is zero-initialized.
void bitmap_create(Bitmap& bitmap, LinearArena& arena, u32 width, u32 height, BitmapFormat format);

void bitmap_destroy(Bitmap& bitmap);

void bitmap_clear(Bitmap& bitmap, BitmapClearColor clear_color);

void bitmap_resize(Bitmap& bitmap, LinearArena& arena, u32 width, u32 height);
