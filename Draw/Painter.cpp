/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#include <Core/Assertion.h>
#include <Core/Math/MathUtils.h>
#include <Core/Memory/MemoryOperations.h>
#include <Draw/Bitmap.h>
#include <Draw/Painter.h>

static Rect get_clamped_rect(const Bitmap* bitmap, Rect quad)
{
    const i32 bl_offset_x = clamp_i32(quad.offset.x, 0, bitmap->width);
    const i32 tr_offset_x = clamp_i32(quad.offset.x + quad.extent.x, 0, bitmap->width);

    const i32 bl_offset_y = clamp_i32(quad.offset.y, 0, bitmap->height);
    const i32 tr_offset_y = clamp_i32(quad.offset.y + quad.extent.y, 0, bitmap->height);

    return rect(bl_offset_x, bl_offset_y, tr_offset_x - bl_offset_x, tr_offset_y - bl_offset_y);
}

void painter_draw_quad(Bitmap* bitmap, Rect quad, LinearColor color)
{
    const Rect clamped_rect = get_clamped_rect(bitmap, quad);
    painter_draw_full_quad(bitmap, clamped_rect, color);
}

void painter_draw_opaque_quad(Bitmap* bitmap, Rect quad, LinearColor color)
{
    const Rect clamped_rect = get_clamped_rect(bitmap, quad);
    painter_draw_full_opaque_quad(bitmap, clamped_rect, color);
}

void painter_draw_full_quad(Bitmap* bitmap, Rect quad, LinearColor color)
{
    // NOTE: Ensure that the quad fits entirely on the bitmap.
    VERIFY(0 <= quad.offset.x && quad.offset.x + quad.extent.x <= bitmap->width);
    VERIFY(0 <= quad.offset.y && quad.offset.y + quad.extent.y <= bitmap->height);

    VERIFY_NOT_REACHED;
}

void painter_draw_full_opaque_quad(Bitmap* bitmap, Rect quad, LinearColor color)
{
    // NOTE: Ensure that the quad fits entirely on the bitmap.
    VERIFY(0 <= quad.offset.x && quad.offset.x + quad.extent.x <= bitmap->width);
    VERIFY(0 <= quad.offset.y && quad.offset.y + quad.extent.y <= bitmap->height);

    if (bitmap->format == BITMAP_FORMAT_B8G8R8A8) {
        const u32 pixel_color = color_pack_linear_to_u32_bgra(linear_color(color.red, color.green, color.blue, 0xFF));
        for (u32 y_offset = quad.offset.y; y_offset < quad.offset.y + quad.extent.y; ++y_offset) {
            u32* row_pixels = (u32*)bitmap_address_of_pixel(bitmap, quad.offset.x, y_offset);
            for (u32 row_it = 0; row_it <= quad.extent.x; ++row_it) {
                *row_pixels = pixel_color;
                ++row_pixels;
            }
        }
    }
    else if (bitmap->format == BITMAP_FORMAT_R8) {
        const usize row_byte_count = bitmap_pixels_row_byte_count(bitmap);
        ReadWriteBytes row = bitmap_address_of_pixel(bitmap, quad.offset.x, quad.offset.y);

        for (usize row_index = 0; row_index < quad.extent.y; ++row_index) {
            set_memory(row, color.red, quad.extent.x);
            row += row_byte_count;
        }
    }
    else {
        VERIFY_NOT_REACHED;
    }
}
