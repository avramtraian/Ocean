/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#include <Core/Assertion.h>
#include <Core/Memory/Arena.h>
#include <Core/Memory/MemoryOperations.h>
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

void bitmap_copy(Bitmap* dst_bitmap, const Bitmap* src_bitmap, BitmapFlip flip /*= BITMAP_FLIP_NONE*/)
{
    VERIFY(dst_bitmap->width == src_bitmap->width);
    VERIFY(dst_bitmap->height == src_bitmap->height);
    VERIFY(dst_bitmap->format == src_bitmap->format);

    if (dst_bitmap->width == 0 || dst_bitmap->height == 0)
        return;

    if (flip == BITMAP_FLIP_NONE) {
        const usize bitmap_byte_count = (usize)dst_bitmap->width * (usize)dst_bitmap->height * bytes_per_pixel_from_bitmap_format(dst_bitmap->format);
        copy_memory(dst_bitmap->pixels, src_bitmap->pixels, bitmap_byte_count);
    }
    else if (flip == BITMAP_FLIP_HORIZONTAL) {
        const usize row_offset = (usize)dst_bitmap->width * bytes_per_pixel_from_bitmap_format(dst_bitmap->format);
        ReadWriteBytes dst_row = bitmap_address_of_pixel(dst_bitmap, 0, 0);
        ReadonlyBytes src_row = bitmap_address_of_pixel(src_bitmap, 0, src_bitmap->height - 1);

        for (u32 row_index = 0; row_index < dst_bitmap->height; ++row_index) {
            copy_memory(dst_row, src_row, row_offset);
            dst_row += row_offset;
            src_row -= row_offset;
        }
    }
    else {
        VERIFY_NOT_REACHED;
    }
}

ReadWriteBytes bitmap_address_of_pixel(const Bitmap* bitmap, u32 x_offset, u32 y_offset)
{
    VERIFY(bitmap->pixels);

    const usize bytes_per_pixel = bytes_per_pixel_from_bitmap_format(bitmap->format);
    return bitmap->pixels + (x_offset + y_offset * bitmap->width) * bytes_per_pixel;
}
