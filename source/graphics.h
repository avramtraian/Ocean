/**
 * Copyright (c) 2025 Traian Avram. All rights reserved.
 * This source file is part of the Ocean text editor.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#pragma once

#include "core.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE(traian): Bitmap.
////////////////////////////////////////////////////////////////////////////////////////////////////

struct Bitmap
{
    void *pixels;
    u32   size_x;
    u32   size_y;
    u32   bytes_per_pixel;
};

function inline usize
bitmap_get_stride(Bitmap *bitmap)
{
    if (bitmap == NULL)
        return 0;

    const usize result = (usize)bitmap->size_x * (usize)bitmap->bytes_per_pixel;
    return result;
}

function inline usize
bitmap_get_pixels_buffer_size(Bitmap *bitmap)
{
    if (bitmap == NULL)
        return 0;

    const usize result = (usize)bitmap->size_y * bitmap_get_stride(bitmap);
    return result;
}

function inline void *
bitmap_get_row_address(Bitmap *bitmap, u32 row_index)
{
    if (bitmap == NULL || row_index > bitmap->size_y)
        return NULL;

    const usize byte_offset = (usize)row_index * bitmap_get_stride(bitmap);
    return (u8 *)bitmap->pixels + byte_offset;
}

function inline void *
bitmap_get_pixel_address(Bitmap *bitmap, u32 offset_x, u32 offset_y)
{
    if (bitmap == NULL || offset_x > bitmap->size_x || offset_y > bitmap->size_y)
        return NULL;

    const usize offset_in_row = (usize)offset_x * (usize)bitmap->bytes_per_pixel;
    return (u8 *)bitmap_get_row_address(bitmap, offset_y) + offset_in_row;
}

function void bitmap_initialize(Bitmap *bitmap, MemoryArena *arena, u32 size_x, u32 size_y, u32 bytes_per_pixel);

////////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE(traian): Font.
////////////////////////////////////////////////////////////////////////////////////////////////////

#define ASCII_CHARACTER_FIRST ((u32)'!')
#define ASCII_CHARACTER_LAST  ((u32)'~')
#define ASCII_CHARACTER_COUNT (ASCII_CHARACTER_LAST - ASCII_CHARACTER_FIRST + 1)

struct FontGlyph
{
    Bitmap bitmap;
    u32    codepoint;
    s32    offset_x;
    s32    offset_y;
};

struct Font
{
    f32       height;
    u32       ascent;
    u32       descent;
    u32       line_gap;
    u32       advance;
    FontGlyph ascii_glyphs[ASCII_CHARACTER_COUNT];
};

function void font_initialize(
    Font *font, MemoryArena *permanent_arena, MemoryArena *work_arena,
    void *ttf_buffer_data, usize ttf_buffer_size, f32 font_height);

function FontGlyph * font_get_glyph(Font *font, u32 codepoint);

////////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE(traian): Tiled text buffer.
////////////////////////////////////////////////////////////////////////////////////////////////////

struct TiledTextCell
{
    u32         codepoint;
    LinearColor color;
};

struct TiledTextBuffer
{
    u32            cell_count_x;
    u32            cell_count_y;
    TiledTextCell *cells;
    u32            cell_size_x;
    u32            cell_size_y;
    u32            line_spacing;
    s32            viewport_offset_x;
    s32            viewport_offset_y;
    u32            viewport_size_x;
    u32            viewport_size_y;
    u32            offset_x;
    u32            offset_y;
};

function void tiled_text_buffer_cell_count_from_viewport(
    u32 viewport_size_x, u32 viewport_size_y,
    u32 cell_size_x, u32 cell_size_y, u32 line_spacing, bool is_offset_allowed,
    u32 *out_cell_count_x, u32 *out_cell_count_y);

function void tiled_text_buffer_initialize(TiledTextBuffer *buffer, MemoryArena *arena, u32 cell_count_x, u32 cell_count_y);
function void tiled_text_buffer_set_cell_size(TiledTextBuffer *buffer, u32 cell_size_x, u32 cell_size_y, u32 line_spacing);

function void tiled_text_buffer_set_viewport(
    TiledTextBuffer *buffer, u32 viewport_offset_x, u32 viewport_offset_y,
    u32 viewport_size_x, u32 viewport_size_y);

function void tiled_text_buffer_set_offset(TiledTextBuffer *buffer, u32 offset_x, u32 offset_y);
function TiledTextCell * tiled_text_buffer_get_cell(TiledTextBuffer *buffer, u32 cell_index_x, u32 cell_index_y);
