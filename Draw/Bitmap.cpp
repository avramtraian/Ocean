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
        case BITMAP_FORMAT_R8G8B8A8: return 4;
    }

    VERIFY_NOT_REACHED;
    return 0;
}

void bitmap_create(Bitmap& bitmap, LinearArena& arena, u32 width, u32 height, BitmapFormat format)
{
    VERIFY(!bitmap.pixels);
    VERIFY(width > 0 && height > 0);

    bitmap.width = width;
    bitmap.height = height;
    bitmap.format = format;

    const usize bitmap_byte_count = width * height * bytes_per_pixel_from_bitmap_format(format);
    bitmap.pixels = (ReadWriteBytes)(core_linear_arena_allocate(arena, bitmap_byte_count));
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

void bitmap_clear(Bitmap& bitmap, BitmapClearColor clear_color)
{
    const usize pixel_count = (usize)(bitmap.width) * (usize)(bitmap.height);

    if (bitmap.format == BITMAP_FORMAT_R8G8B8A8) {
        const u8 red = (u8)(clear_color.red * 255.0F);
        const u8 green = (u8)(clear_color.green * 255.0F);
        const u8 blue = (u8)(clear_color.blue * 255.0F);
        const u8 alpha = (u8)(clear_color.alpha * 255.0F);
        const u32 pixel = (red << 24) | (green << 16) | (blue << 8) | (alpha << 0);

        u32* current_pixel = (u32*)(bitmap.pixels);
        for (usize pixel_index = 0; pixel_index < pixel_count; ++pixel_index) {
            *current_pixel = pixel;
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
