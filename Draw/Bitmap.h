/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#pragma once

#include <Core/Forward.h>
#include <Core/Math/Color.h>
#include <Core/Types.h>

enum BitmapFormatEnum {
    BITMAP_FORMAT_UNKNOWN = 0,
    BITMAP_FORMAT_B8G8R8A8,
};
typedef u8 BitmapFormat;

typedef struct Bitmap {
    u32 width;
    u32 height;
    ReadWriteBytes pixels;
    BitmapFormat format;
} Bitmap;

// NOTE: The pixels are all black, as the memory returned by the arena is zero-initialized.
void bitmap_create(Bitmap& bitmap, LinearArena& arena, u32 width, u32 height, BitmapFormat format);

void bitmap_create_from_memory(Bitmap* bitmap, u32 width, u32 height, BitmapFormat format, ReadWriteBytes pixels);

void bitmap_destroy(Bitmap& bitmap);

void bitmap_clear(Bitmap& bitmap, LinearColor clear_color);

void bitmap_resize(Bitmap& bitmap, LinearArena& arena, u32 width, u32 height);

ReadWriteBytes bitmap_address_of_pixel(const Bitmap* bitmap, u32 x_offset, u32 y_offset);
