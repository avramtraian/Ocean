/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#pragma once

#include <Core/Math/Color.h>
#include <Core/Memory/Arena.h>
#include <Core/Memory/MemorySpan.h>
#include <Core/Types.h>

// Opaque handle towards a bitmap.
typedef void* Bitmap;

enum BitmapFormatEnum {
    BITMAP_FORMAT_UNKNOWN = 0,
    BITMAP_FORMAT_B8G8R8A8,
    BITMAP_FORMAT_R8,
};
typedef u8 BitmapFormat;

enum BitmapFlipBitsEnum {
    BITMAP_FLIP_NONE = 0,
    BITMAP_FLIP_HORIZONTAL = 1 << 0,
    BITMAP_FLIP_VERTICAL = 1 << 1,
};
typedef u8 BitmapFlipBits;

Bitmap bitmap_create(LinearArena* arena, u32 width, u32 height, BitmapFormat format);

// NOTE: The provided memory byte span must contain exactly enough bytes to store the bitmap pixel data.
Bitmap bitmap_create_from_memory(LinearArena* arena, u32 width, u32 height, BitmapFormat format, ReadWriteByteSpan memory_buffer);

void bitmap_destroy(Bitmap* bitmap_handle);

u32 bitmap_get_width(Bitmap bitmap_handle);

u32 bitmap_get_height(Bitmap bitmap_handle);

BitmapFormat bitmap_get_format(Bitmap bitmap_handle);

u32 bitmap_get_bits_per_pixel(Bitmap bitmap_handle);
// NOTE: If the value is not a whole number zero will be return. Use 'bitmap_get_bits_per_pixel' in this case.
u32 bitmap_get_bytes_per_pixel(Bitmap bitmap_handle);

usize bitmap_get_pitch(Bitmap bitmap_handle);

usize bitmap_get_byte_count(Bitmap bitmap_handle);

// NOTE: Depending on the underlaying bitmap type the performance of this function might vary significantly.
ReadWriteBytes bitmap_get_data(Bitmap bitmap_handle);

// NOTE: Depending on the underlaying bitmap type the performance of this function might vary significantly.
ReadWriteBytes bitmap_get_pixel_address(Bitmap bitmap_handle, u32 offset_x, u32 offset_y);

// NOTE: The destination and source bitmaps must have the same width, height and format.
void bitmap_copy(Bitmap destination_bitmap_handle, Bitmap source_bitmap_handle, BitmapFlipBits flip_bits = BITMAP_FLIP_NONE);

void bitmap_clear(Bitmap bitmap_handle, LinearColor clear_color = linear_color(0, 0, 0, 0));

// NOTE: No data will be copied during the resize operation.
void bitmap_resize(Bitmap bitmap_handle, LinearArena* arena, u32 new_width, u32 new_height);

// NOTE: No data will be copied during the resize operation.
//       The provided memory byte span must contain exactly enough bytes to store the bitmap pixel data.
void bitmap_resize_from_memory(Bitmap bitmap_handle, u32 new_width, u32 new_height, ReadWriteByteSpan new_memory_buffer);
