/**
 * Copyright (c) 2025 Traian Avram. All rights reserved.
 * This source file is part of the Ocean text editor.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#include "graphics.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE(traian): Bitmap.
////////////////////////////////////////////////////////////////////////////////////////////////////

function void
bitmap_initialize(Bitmap *bitmap, MemoryArena *arena, u32 size_x, u32 size_y, u32 bytes_per_pixel)
{
    if (bitmap == NULL)
        return;

    if (arena) {
        bitmap->size_x = size_x;
        bitmap->size_y = size_y;
        bitmap->bytes_per_pixel = bytes_per_pixel;
        bitmap->pixels = memory_arena_allocate(arena, bitmap_get_pixels_buffer_size(bitmap));
    }
    else {
        bitmap->pixels = NULL;
        bitmap->size_x = 0;
        bitmap->size_y = 0;
        bitmap->bytes_per_pixel = 0;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE(traian): Font.
////////////////////////////////////////////////////////////////////////////////////////////////////

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

function void
font_initialize(Font *font, MemoryArena *permanent_arena,
                void *ttf_buffer_data, usize ttf_buffer_size, f32 font_height)
{
    stbtt_fontinfo font_info;
    stbtt_InitFont(&font_info, (u8 *)ttf_buffer_data, stbtt_GetFontOffsetForIndex((u8 *)ttf_buffer_data, 0));
    f32 scale = stbtt_ScaleForPixelHeight(&font_info, font_height);

    int advance;
    stbtt_GetCodepointHMetrics(&font_info, 'X', &advance, NULL);
    font->advance = (u32)((f32)advance * scale);

    int ascent, descent, line_gap;
    stbtt_GetFontVMetrics(&font_info, &ascent, &descent, &line_gap);
    font->ascent = (u32)((f32)ascent * scale);
    font->descent = (u32)((f32)-descent * scale);
    font->line_gap = (u32)((f32)line_gap * scale);

    // NOTE(traian): Load the glyphs for all visible ASCII codepoints.
    for (u32 ascii_codepoint_index = 0; ascii_codepoint_index < ASCII_CHARACTER_COUNT; ++ascii_codepoint_index) {
        const u32 codepoint = ASCII_CHARACTER_FIRST + ascii_codepoint_index;
        FontGlyph *glyph = font->ascii_glyphs + ascii_codepoint_index;
        
        // TODO(traian): Figure it out how 'stbtt_GetCodepointBitmapBox' works and just rasterize the glyph
        // directly into our own bitmap (and flip it afterwards), instead of allocating temporary memory from the heap.
        // The current behaviour defeats the entire purpose of the work memory arena! - 30 Mar 2025
        int size_x, size_y, offset_x, offset_y;
        u8 *flipped_bitmap_data = stbtt_GetCodepointBitmap(&font_info, scale, scale, codepoint, &size_x, &size_y, &offset_x, &offset_y);

        bitmap_initialize(&glyph->bitmap, permanent_arena, size_x, size_y, 1);

        if (size_x > 0 && size_y > 0) {
            u8 *dst_row = (u8 *)bitmap_get_row_address(&glyph->bitmap, size_y - 1);
            u8 *src_row = flipped_bitmap_data;

            for (u32 y = 0; y < (u32)size_y; ++y) {
                for (u32 x = 0; x < (u32)size_x; ++x) {
                    dst_row[x] = src_row[x];
                }

                dst_row -= size_x;
                src_row += size_x;
            }
        }

        glyph->codepoint = codepoint;
        glyph->offset_x = offset_x;
        glyph->offset_y = -(size_y + offset_y);

        stbtt_FreeBitmap(flipped_bitmap_data, NULL);
    }
}

function FontGlyph *
font_get_glyph(Font *font, u32 codepoint)
{
    if (ASCII_CHARACTER_FIRST <= codepoint && codepoint <= ASCII_CHARACTER_LAST) {
        const u32 glyph_index = codepoint - ASCII_CHARACTER_FIRST;
        return font->ascii_glyphs + glyph_index;
    }

    // TODO(traian): Support extended Unicode pages!
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE(traian): Tiled text buffer.
////////////////////////////////////////////////////////////////////////////////////////////////////

function void
tiled_text_buffer_cell_count_from_viewport(u32 viewport_size_x, u32 viewport_size_y,
                                           u32 cell_size_x, u32 cell_size_y, u32 line_spacing, bool is_offset_allowed,
                                           u32 *out_cell_count_x, u32 *out_cell_count_y)
{
    u32 viewport_with_offset_x = viewport_size_x;
    u32 viewport_with_offset_y = viewport_size_y;
    if (is_offset_allowed) {
        viewport_with_offset_x += (cell_size_x - 1);
        viewport_with_offset_y += (cell_size_y - 1);
    }

    *out_cell_count_x = required_to_fill_u32(cell_size_x, viewport_with_offset_x);
    *out_cell_count_y = required_to_fill_u32(cell_size_y + line_spacing, viewport_with_offset_y + line_spacing);
}

function void
tiled_text_buffer_initialize(TiledTextBuffer *buffer, MemoryArena *arena, u32 cell_count_x, u32 cell_count_y)
{
    if (buffer == NULL)
        return;

    buffer->cell_count_x = cell_count_x;
    buffer->cell_count_y = cell_count_y;
    buffer->cells = ARENA_PUSH_ARRAY(arena, TiledTextCell, cell_count_x * cell_count_y);
    buffer->cell_size_x = 0;
    buffer->cell_size_y = 0;
    buffer->viewport_offset_x = 0;
    buffer->viewport_offset_y = 0;
    buffer->viewport_size_x = 0;
    buffer->viewport_size_y = 0;
    buffer->offset_x = 0;
    buffer->offset_y = 0;
}

function void
tiled_text_buffer_set_cell_size(TiledTextBuffer *buffer, u32 cell_size_x, u32 cell_size_y, u32 line_spacing)
{
    if (buffer == NULL)
        return;

    buffer->cell_size_x = cell_size_x;
    buffer->cell_size_y = cell_size_y;
    buffer->line_spacing = line_spacing;
}

function void
tiled_text_buffer_set_viewport(TiledTextBuffer *buffer, u32 viewport_offset_x, u32 viewport_offset_y,
                               u32 viewport_size_x, u32 viewport_size_y)
{
    if (buffer == NULL)
        return;

    buffer->viewport_offset_x = viewport_offset_x;
    buffer->viewport_offset_y = viewport_offset_y;
    buffer->viewport_size_x = viewport_size_x;
    buffer->viewport_size_y = viewport_size_y;
}

function void
tiled_text_buffer_set_offset(TiledTextBuffer *buffer, u32 offset_x, u32 offset_y)
{
    if (buffer == NULL)
        return;

    buffer->offset_x = offset_x;
    buffer->offset_y = offset_y;
}

function void
tiled_text_buffer_initialize_from_font_and_viewport(TiledTextBuffer *buffer, MemoryArena *arena,
    u32 viewport_offset_x, u32 viewport_offset_y, u32 viewport_size_x, u32 viewport_size_y,
    Font *font, bool is_offset_allowed)
{
    const u32 cell_size_x = font->advance;
    const u32 cell_size_y = font->ascent + font->descent;
    const u32 line_spacing = font->line_gap;

    u32 cell_count_x, cell_count_y;
    tiled_text_buffer_cell_count_from_viewport(
        viewport_size_x, viewport_size_y, cell_size_x, cell_size_y, line_spacing, is_offset_allowed,
        &cell_count_x, &cell_count_y);

    tiled_text_buffer_initialize(buffer, arena, cell_count_x, cell_count_y);
    tiled_text_buffer_set_cell_size(buffer, cell_size_x, cell_size_y, line_spacing);
    tiled_text_buffer_set_viewport(buffer, viewport_offset_x, viewport_offset_y, viewport_size_x, viewport_size_y);
    tiled_text_buffer_set_offset(buffer, 0, 0);
}

function TiledTextCell *
tiled_text_buffer_get_cell(TiledTextBuffer *buffer, u32 cell_index_x, u32 cell_index_y)
{
    if (buffer == NULL)
        return NULL;
    if (cell_index_x > buffer->cell_count_x || cell_index_y > buffer->cell_count_y)
        return NULL;

    const u32 cell_index = cell_index_x + (cell_index_y * buffer->cell_count_x);
    return buffer->cells + cell_index;
}
