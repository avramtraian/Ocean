/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#include <Core/Assertion.h>
#include <Core/Memory/MemoryOperations.h>
#include <Draw/Bitmap.h>

typedef struct SoftwareBitmap {
    u32 width;
    u32 height;
    BitmapFormat format;
    ReadWriteBytes data;
} SoftwareBitmap;

Bitmap bitmap_create(LinearArena* arena, u32 width, u32 height, BitmapFormat format)
{
    SoftwareBitmap* bitmap = (SoftwareBitmap*)core_linear_arena_allocate(*arena, sizeof(SoftwareBitmap));

    bitmap->width = width;
    bitmap->height = height;
    bitmap->format = format;
    bitmap->data = (ReadWriteBytes)core_linear_arena_allocate(*arena, bitmap_get_byte_count(bitmap));

    return bitmap;
}

Bitmap bitmap_create_from_memory(LinearArena* arena, u32 width, u32 height, BitmapFormat format, ReadWriteByteSpan memory_buffer)
{
    SoftwareBitmap* bitmap = (SoftwareBitmap*)core_linear_arena_allocate(*arena, sizeof(SoftwareBitmap));

    bitmap->width = width;
    bitmap->height = height;
    bitmap->format = format;
    bitmap->data = memory_buffer.bytes;

    VERIFY(memory_buffer.count == bitmap_get_byte_count(bitmap));
    return bitmap;
}

void bitmap_destroy(Bitmap* bitmap_handle)
{
    SoftwareBitmap* bitmap = (SoftwareBitmap*)(*bitmap_handle);
    bitmap->width = 0;
    bitmap->height = 0;
    bitmap->format = BITMAP_FORMAT_UNKNOWN;
    bitmap->data = nullptr;

    *bitmap_handle = INVALID_HANDLE;
}

u32 bitmap_get_width(Bitmap bitmap_handle)
{
    SoftwareBitmap* bitmap = (SoftwareBitmap*)bitmap_handle;
    return bitmap->width;
}

u32 bitmap_get_height(Bitmap bitmap_handle)
{
    SoftwareBitmap* bitmap = (SoftwareBitmap*)bitmap_handle;
    return bitmap->height;
}

BitmapFormat bitmap_get_format(Bitmap bitmap_handle)
{
    SoftwareBitmap* bitmap = (SoftwareBitmap*)bitmap_handle;
    return bitmap->format;
}

u32 bitmap_get_bits_per_pixel(Bitmap bitmap_handle)
{
    SoftwareBitmap* bitmap = (SoftwareBitmap*)bitmap_handle;
    switch (bitmap->format) {
        case BITMAP_FORMAT_UNKNOWN: return 0;
        case BITMAP_FORMAT_B8G8R8A8: return 32;
        case BITMAP_FORMAT_R8: return 8;
    }

    VERIFY_NOT_REACHED;
    return 0;
}

u32 bitmap_get_bytes_per_pixel(Bitmap bitmap_handle)
{
    const u32 bits_per_pixel = bitmap_get_bits_per_pixel(bitmap_handle);
    if (bits_per_pixel % 8 != 0)
        return 0;
    return bits_per_pixel / 8;
}

usize bitmap_get_pitch(Bitmap bitmap_handle)
{
    const usize pitch_bits = (usize)bitmap_get_width(bitmap_handle) * (usize)bitmap_get_bits_per_pixel(bitmap_handle);
    VERIFY(pitch_bits % 8 == 0);
    return pitch_bits / 8;
}

usize bitmap_get_byte_count(Bitmap bitmap_handle)
{
    return (usize)bitmap_get_height(bitmap_handle) * bitmap_get_pitch(bitmap_handle);
}

ReadWriteBytes bitmap_get_data(Bitmap bitmap_handle)
{
    SoftwareBitmap* bitmap = (SoftwareBitmap*)bitmap_handle;
    return bitmap->data;
}

ReadWriteBytes bitmap_get_pixel_address(Bitmap bitmap_handle, u32 offset_x, u32 offset_y)
{
    const usize bytes_per_pixel = bitmap_get_bytes_per_pixel(bitmap_handle);
    VERIFY(bytes_per_pixel != 0);

    SoftwareBitmap* bitmap = (SoftwareBitmap*)bitmap_handle;
    return bitmap->data + (offset_x + offset_y * bitmap->width) * bytes_per_pixel;
}

void bitmap_copy(Bitmap destination_bitmap_handle, Bitmap source_bitmap_handle, BitmapFlipBits flip_bits /*= BITMAP_FLIP_NONE*/)
{
    SoftwareBitmap* destination_bitmap = (SoftwareBitmap*)destination_bitmap_handle;
    SoftwareBitmap* source_bitmap = (SoftwareBitmap*)source_bitmap_handle;

    VERIFY(destination_bitmap->width == source_bitmap->width);
    VERIFY(destination_bitmap->height == source_bitmap->height);
    VERIFY(destination_bitmap->format == source_bitmap->format);

    const usize bitmap_byte_count = (usize)destination_bitmap->height * bitmap_get_pitch(destination_bitmap_handle);
    copy_memory(destination_bitmap->data, source_bitmap->data, bitmap_byte_count);
}

void bitmap_clear(Bitmap bitmap_handle, LinearColor clear_color /*= linear_color(0, 0, 0, 0)*/)
{
    SoftwareBitmap* bitmap = (SoftwareBitmap*)bitmap_handle;
    switch (bitmap->format) {
        case BITMAP_FORMAT_B8G8R8A8: {
            u32* current_pixel = (u32*)bitmap->data;
            const u32 pixel_color = color_pack_linear_to_u32_bgra(clear_color);
            const usize pixel_count = (usize)bitmap->width * (usize)bitmap->height;

            for (usize pixel_index = 0; pixel_index < pixel_count; ++pixel_index) {
                *current_pixel = pixel_color;
                ++current_pixel;
            }
            break;
        }

        case BITMAP_FORMAT_R8: {
            set_memory(bitmap->data, clear_color.red, bitmap_get_byte_count(bitmap_handle));
            break;
        }

        default: {
            VERIFY_NOT_REACHED;
            break;
        }
    }
}

void bitmap_resize(Bitmap bitmap_handle, LinearArena* arena, u32 new_width, u32 new_height)
{
    SoftwareBitmap* bitmap = (SoftwareBitmap*)bitmap_handle;
    bitmap->width = new_width;
    bitmap->height = new_height;
    bitmap->data = (ReadWriteBytes)core_linear_arena_allocate(*arena, bitmap_get_byte_count(bitmap_handle));
}

void bitmap_resize_from_memory(Bitmap bitmap_handle, u32 new_width, u32 new_height, ReadWriteByteSpan new_memory_buffer)
{
    SoftwareBitmap* bitmap = (SoftwareBitmap*)bitmap_handle;
    bitmap->width = new_width;
    bitmap->height = new_height;
    bitmap->data = new_memory_buffer.bytes;
    VERIFY(bitmap_get_byte_count(bitmap_handle) == new_memory_buffer.count);
}
