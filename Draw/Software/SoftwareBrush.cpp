/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#include <Core/Assertion.h>
#include <Core/Math/MathUtils.h>
#include <Core/Math/Rect.h>
#include <Core/Memory/MemoryOperations.h>
#include <Draw/Brush.h>

typedef struct SoftwareBrush {
} SoftwareBrush;

Brush brush_create(LinearArena* arena)
{
    SoftwareBrush* brush = (SoftwareBrush*)core_linear_arena_allocate(*arena, sizeof(SoftwareBrush));
    return brush;
}

void brush_destroy(Brush* brush_handle)
{
    *brush_handle = INVALID_HANDLE;
}

static Rect get_clamped_rect(Bitmap bitmap_handle, Rect clamped_rect)
{
    const i32 bottom_left_x = clamp_i32(clamped_rect.offset.x, 0, bitmap_get_width(bitmap_handle));
    const i32 bottom_left_y = clamp_i32(clamped_rect.offset.y, 0, bitmap_get_height(bitmap_handle));

    const i32 top_right_x = clamp_i32(clamped_rect.offset.x + clamped_rect.extent.x, 0, bitmap_get_width(bitmap_handle));
    const i32 top_right_y = clamp_i32(clamped_rect.offset.y + clamped_rect.extent.y, 0, bitmap_get_height(bitmap_handle));

    return rect(bottom_left_x, bottom_left_y, top_right_x - bottom_left_x, top_right_y - bottom_left_y);
}

void brush_draw_opaque_quad(Brush, Bitmap bitmap_handle, Rect quad, LinearColor color)
{
    // NOTE: Clamp the quad rect to the bitmap dimensions.
    quad = get_clamped_rect(bitmap_handle, quad);

    switch (bitmap_get_format(bitmap_handle)) {
        case BITMAP_FORMAT_B8G8R8A8: {
            const u32 pixel_color = color_pack_linear_to_u32_bgra(color);
            u32* current_pixel = (u32*)bitmap_get_pixel_address(bitmap_handle, quad.offset.x, quad.offset.y);
            const u32 bitmap_pixel_jump_count = bitmap_get_width(bitmap_handle) - quad.extent.x;

            for (usize offset_y = 0; offset_y < quad.extent.y; ++offset_y) {
                for (usize offset_x = 0; offset_x < quad.extent.x; ++offset_x) {
                    *current_pixel = pixel_color;
                    ++current_pixel;
                }

                current_pixel += bitmap_pixel_jump_count;
            }

            break;
        }

        case BITMAP_FORMAT_R8: {
            ReadWriteBytes current_row = bitmap_get_pixel_address(bitmap_handle, quad.offset.x, quad.offset.y);
            const usize bitmap_pitch = bitmap_get_pitch(bitmap_handle);

            for (usize offset_y = 0; offset_y <= quad.extent.y; ++offset_y) {
                set_memory(current_row, color.red, quad.extent.x);
                current_row += bitmap_pitch;
            }
            break;
        }

        default: {
            VERIFY_NOT_REACHED;
            break;
        }
    }
}

void brush_draw_opaque_bitmap(Brush, Bitmap bitmap_handle, Bitmap bitmap_to_draw_handle, Vector2i offset)
{
    VERIFY(0 <= offset.x && offset.x + bitmap_get_width(bitmap_to_draw_handle) <= bitmap_get_width(bitmap_handle));
    VERIFY(0 <= offset.y && offset.y + bitmap_get_height(bitmap_to_draw_handle) <= bitmap_get_height(bitmap_handle));

    if (bitmap_get_format(bitmap_handle) == bitmap_get_format(bitmap_to_draw_handle)) {
        ReadWriteBytes dst_bitmap_row = bitmap_get_pixel_address(bitmap_handle, offset.x, offset.y);
        ReadWriteBytes src_bitmap_row = bitmap_get_data(bitmap_to_draw_handle);

        const usize dst_bitmap_pitch = bitmap_get_pitch(bitmap_handle);
        const usize src_bitmap_pitch = bitmap_get_pitch(bitmap_to_draw_handle);

        for (usize offset_y = 0; offset_y < bitmap_get_height(bitmap_to_draw_handle); ++offset_y) {
            copy_memory(dst_bitmap_row, src_bitmap_row, src_bitmap_pitch);
            dst_bitmap_row += dst_bitmap_pitch;
            src_bitmap_row += src_bitmap_pitch;
        }
    }
    else {
        // TODO: Implement me!
        VERIFY_NOT_REACHED;
    }
}

void brush_draw_font_bitmap(Brush brush_handle, Bitmap bitmap_handle, Bitmap font_bitmap_handle, Vector2i offset, LinearColor font_color)
{
    VERIFY(0 <= offset.x && offset.x + bitmap_get_width(font_bitmap_handle) <= bitmap_get_width(bitmap_handle));
    VERIFY(0 <= offset.y && offset.y + bitmap_get_height(font_bitmap_handle) <= bitmap_get_height(bitmap_handle));

    // NOTE: This is the only supported font bitmap format.
    VERIFY(bitmap_get_format(font_bitmap_handle) == BITMAP_FORMAT_R8);

    switch (bitmap_get_format(bitmap_handle)) {
        case BITMAP_FORMAT_B8G8R8A8: {
            const u32 font_bitmap_width = bitmap_get_width(font_bitmap_handle);
            const u32 font_bitmap_height = bitmap_get_height(font_bitmap_handle);

            u32* dst_current_pixel = (u32*)bitmap_get_pixel_address(bitmap_handle, offset.x, offset.y);
            ReadWriteBytes src_current_pixel = bitmap_get_data(font_bitmap_handle);
            const u32 dst_pixel_jump_count = bitmap_get_width(bitmap_handle) - font_bitmap_width;

            for (usize offset_y = 0; offset_y < font_bitmap_height; ++offset_y) {
                for (usize offset_x = 0; offset_x < font_bitmap_width; ++offset_x) {
                    const LinearColor current_color = color_unpack_linear_from_u32_bgra(*dst_current_pixel);
                    const LinearColor blended_color = color_blend_linear_colors(current_color, font_color, *src_current_pixel);
                    *dst_current_pixel = color_pack_linear_to_u32_bgra(blended_color);

                    ++dst_current_pixel;
                    ++src_current_pixel;
                }

                dst_current_pixel += dst_pixel_jump_count;
            }

            break;
        }

        default: {
            VERIFY_NOT_REACHED;
            break;
        }
    }
}
