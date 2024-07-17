/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#include <Core/Assertion.h>
#include <Core/Memory/Arena.h>
#include <Draw/Bitmap.h>

static usize bytes_per_pixel_from_bitmap_format(BitmapFormat format)
{
    switch (format) {
        case BITMAP_FORMAT_B8G8R8A8: return 4;
    }

    VERIFY_NOT_REACHED;
    return 0;
}

void bitmap_create(Bitmap& bitmap, LinearArena& arena, u32 width, u32 height, BitmapFormat format)
{
    const usize bitmap_byte_count = width * height * bytes_per_pixel_from_bitmap_format(format);
    ReadWriteBytes pixels = (ReadWriteBytes)(core_linear_arena_allocate(arena, bitmap_byte_count));
    bitmap_create_from_memory(&bitmap, width, height, format, pixels);
}

void bitmap_create_from_memory(Bitmap* bitmap, u32 width, u32 height, BitmapFormat format, ReadWriteBytes pixels)
{
    VERIFY(!bitmap->pixels);
    VERIFY(width > 0 && height > 0);

    bitmap->width = width;
    bitmap->height = height;
    bitmap->format = format;
    bitmap->pixels = pixels;
}

void bitmap_destroy(Bitmap& bitmap)
{
    bitmap.width = 0;
    bitmap.height = 0;
    bitmap.format = BITMAP_FORMAT_UNKNOWN;

    // NOTE: As the memory is allocated from a linear arena, which doesn't support freeing, this operation
    //       doesn't represent a memory leak.
    bitmap.pixels = nullptr;
}

void bitmap_clear(Bitmap& bitmap, LinearColor clear_color)
{
    const usize pixel_count = (usize)(bitmap.width) * (usize)(bitmap.height);

    if (bitmap.format == BITMAP_FORMAT_B8G8R8A8) {
        const u32 pixel_color = color_pack_linear_to_u32_bgra(clear_color);
        u32* current_pixel = (u32*)(bitmap.pixels);

        for (usize pixel_index = 0; pixel_index < pixel_count; ++pixel_index) {
            *current_pixel = pixel_color;
            ++current_pixel;
        }
    }
    else {
        VERIFY_NOT_REACHED;
    }
}

void bitmap_resize(Bitmap& bitmap, LinearArena& arena, u32 width, u32 height)
{
    const BitmapFormat format = bitmap.format;
    bitmap_destroy(bitmap);
    bitmap_create(bitmap, arena, width, height, format);
}
