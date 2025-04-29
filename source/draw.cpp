/**
 * Copyright (c) 2025 Traian Avram. All rights reserved.
 * This source file is part of the Ocean text editor.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#include "draw.h"

function void
draw_clear_bitmap(Bitmap *bitmap, LinearColor clear_color)
{
    if (bitmap == NULL)
        return;

    if (bitmap->bytes_per_pixel == 4) {
        const u32 packed_color = linear_color_pack_to_u32(clear_color);
        u32 *dst_row = (u32 *)bitmap->pixels;
        
        for (u32 y = 0; y < bitmap->size_y; ++y) {
            for (u32 x = 0; x < bitmap->size_x; ++x) {
                dst_row[x] = packed_color;
            }
            dst_row += bitmap->size_x;
        }
    }
    else {
        // TODO(traian): Implement!
        ASSERT_NOT_REACHED;
    }
}

function void
draw_quad(Bitmap *bitmap, s32 offset_x, s32 offset_y, u32 size_x, u32 size_y, LinearColor color)
{
    if (bitmap == NULL)
        return;

    offset_x = clamp_s32(offset_x, 0, bitmap->size_x);
    offset_y = clamp_s32(offset_y, 0, bitmap->size_y);
    const u32 max_x = clamp_s32(offset_x + size_x, 0, bitmap->size_x);
    const u32 max_y = clamp_s32(offset_y + size_y, 0, bitmap->size_y);
    size_x = max_x - offset_x;
    size_y = max_y - offset_y;

    if (bitmap->bytes_per_pixel == 4) {
        const u32 packed_color = linear_color_pack_to_u32(color);
        u32 *dst_row = (u32 *)bitmap_get_pixel_address(bitmap, offset_x, offset_y);

        for (u32 y = 0; y < size_y; ++y) {
            for (u32 x = 0; x < size_x; ++x) {
                dst_row[x] = packed_color;
            }
            dst_row += bitmap->size_x;
        }
    }
    else {
        // TODO(traian): Implement!
        ASSERT_NOT_REACHED;
    }
}

function void
draw_rectangle(Bitmap *bitmap, s32 offset_x, s32 offset_y, u32 size_x, u32 size_y, u32 thickness, LinearColor color)
{
    const u32 thickness_x = thickness;
    const u32 thickness_y = thickness;

    // NOTE(traian): Bottom.
    draw_quad(bitmap,
        offset_x, offset_y,
        size_x - thickness_x, thickness_y,
        color);

    // NOTE(traian): Right.
    draw_quad(bitmap,
        offset_x + size_x - thickness_x, offset_y,
        thickness_x, size_y - thickness_y,
        color);

    // NOTE(traian): Top.
    draw_quad(bitmap,
        offset_x + thickness_x, offset_y + size_y - thickness_y,
        size_x - thickness_x, thickness_y,
        color);

    // NOTE(traian): Left.
    draw_quad(bitmap,
        offset_x, offset_y + thickness_y,
        thickness_x, size_y - thickness_y,
        color);
}

function void
draw_rectangle_containing(Bitmap *bitmap, s32 offset_x, s32 offset_y, u32 size_x, u32 size_y, u32 thickness, LinearColor color)
{
    draw_rectangle(bitmap,
                   offset_x - thickness, offset_y - thickness,
                   size_x + 2 * thickness, size_y + 2 * thickness,
                   thickness, color);
}

function void
draw_glyph_bitmap(Bitmap *bitmap, Bitmap *glyph_bitmap, s32 offset_x, s32 offset_y, LinearColor color,
                  u32 viewport_offset_x, u32 viewport_offset_y, u32 viewport_size_x, u32 viewport_size_y)
{
    if (bitmap == NULL || glyph_bitmap == NULL)
        return;
    ASSERT(glyph_bitmap->bytes_per_pixel == 1);

    // NOTE(traian): An invalid viewport rectangle was provided.
    if (viewport_offset_x + viewport_size_x > bitmap->size_x || viewport_offset_y + viewport_size_y > bitmap->size_y)
        return;

    Rect2s viewport_rect = rect2s(viewport_offset_x, viewport_offset_y, 
                                  viewport_offset_x + viewport_size_x,
                                  viewport_offset_y + viewport_size_y);
    Rect2s glyph_rect = rect2s(offset_x, offset_y,
                               offset_x + glyph_bitmap->size_x, offset_y + glyph_bitmap->size_y);
    Rect2s intersection_rect = rect2_intersect(viewport_rect, glyph_rect);
    if (rect2_is_degenerated(intersection_rect))
        return;

    const u32 glyph_offset_x = intersection_rect.min_x - glyph_rect.min_x;
    const u32 glyph_offset_y = intersection_rect.min_y - glyph_rect.min_y;
    u32 glyph_size_x, glyph_size_y;
    rect2_size(intersection_rect, &glyph_size_x, &glyph_size_y);

    if (bitmap->bytes_per_pixel == 4) {
        u32 *dst_row = (u32 *)bitmap_get_pixel_address(bitmap, intersection_rect.min_x, intersection_rect.min_y);
        u8 *src_row = (u8 *)bitmap_get_pixel_address(glyph_bitmap, glyph_offset_x, glyph_offset_y);

        for (u32 y = 0; y < glyph_size_y; ++y) {
            for (u32 x = 0; x < glyph_size_x; ++x) {
                const LinearColor current_color = linear_color_unpack_from_u32(dst_row[x]);
                const f32 alpha = (f32)src_row[x] / 255.0F;
                const LinearColor blended_color = linear_color_blend(current_color, color, alpha);
                dst_row[x] = linear_color_pack_to_u32(blended_color);
            }

            dst_row += bitmap->size_x;
            src_row += glyph_bitmap->size_x;
        }
    }
    else {
        // TODO(traian): Implement!
        ASSERT_NOT_REACHED;
    }
}

function void
draw_tiled_text_buffer(Bitmap *bitmap, TiledTextBuffer *buffer, Font *font)
{
    if (bitmap == NULL || buffer == NULL)
        return;

    // NOTE(traian): Ensure no trivial invalid state.
    if (buffer->cell_size_x == 0 || buffer->cell_size_y == 0 || buffer->cell_count_x == 0 || buffer->cell_count_y == 0)
        return;
    if (buffer->viewport_size_x == 0 || buffer->viewport_size_y == 0)
        return;

    const u32 size_with_offset_x = buffer->viewport_size_x + buffer->offset_x;
    const u32 size_with_offset_y = buffer->viewport_size_y + buffer->offset_y;
    const u32 required_to_fill_x = required_to_fill_u32(buffer->cell_size_x, size_with_offset_x);
    const u32 required_to_fill_y = required_to_fill_u32(buffer->cell_size_y + buffer->line_spacing, size_with_offset_y + buffer->line_spacing);

    s32 base_cell_offset_x = buffer->viewport_offset_x - buffer->offset_x;
    s32 base_cell_offset_y = buffer->viewport_offset_y + buffer->viewport_size_y + buffer->offset_y - buffer->cell_size_y;

    for (u32 cell_index_y = 0; cell_index_y < required_to_fill_y; ++cell_index_y) {
        s32 cell_offset_x = base_cell_offset_x;
        s32 cell_offset_y = base_cell_offset_y - (cell_index_y * (buffer->cell_size_y + buffer->line_spacing));
        
        for (u32 cell_index_x = 0; cell_index_x < required_to_fill_x; ++cell_index_x) {
            TiledTextCell *cell = tiled_text_buffer_get_cell(buffer, cell_index_x, cell_index_y);
            if (ASCII_CHARACTER_FIRST <= cell->codepoint && cell->codepoint <= ASCII_CHARACTER_LAST) {
                FontGlyph *glyph = font_get_glyph(font, cell->codepoint);
                draw_glyph_bitmap(bitmap, &glyph->bitmap,
                    cell_offset_x + glyph->offset_x, cell_offset_y + glyph->offset_y + font->descent, cell->color,
                    buffer->viewport_offset_x, buffer->viewport_offset_y,
                    buffer->viewport_size_x, buffer->viewport_size_y);
            }

            cell_offset_x += buffer->cell_size_x;
        }
    }
}
