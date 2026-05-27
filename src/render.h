// Copyright (c) 2026 Traian Avram. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause.

#ifndef RENDER_H
#define RENDER_H

#include "core.h"
#include "math_utils.h"

//============================================================================//
//---------------------------------- BITMAP ----------------------------------//
//============================================================================//

enum bitmap_format : u8
{
    BitmapFormat_Unknown = 0,
    BitmapFormat_BGRA_8888,
    BitmapFormat_R_8,
};

internal usize GetFormatBytesPerPixel(bitmap_format Format);

struct bitmap
{
    u8*           Data;
    u32           Width;
    u32           Height;
    bitmap_format Format;
};

internal usize GetBytesPerPixel(bitmap* Bitmap);
internal usize GetBytesPerRow(bitmap* Bitmap);
internal rect2s GetBitmapRect(bitmap* Bitmap);

internal void* GetPixelAddress(bitmap* Bitmap, u32 OffsetX, u32 OffsetY);

//============================================================================//
//----------------------------------- FONT -----------------------------------//
//============================================================================//

struct font_glyph
{
    u32    CodePoint;
    bitmap Bitmap;
};

struct font
{
};

internal b8 LoadFontFromMemory(font* Font, memory_buffer Buffer);

internal font_glyph* GetFontGlyph(font* Font, u32 CodePoint);

//============================================================================//
//--------------------------------- TEXT GRID --------------------------------//
//============================================================================//

typedef u8 text_style_bits;
enum text_style_enum : text_style_bits
{
    TextStyle_Regular    = 0,
    TextStyle_Bold       = 1 << 0,
    TextStyle_Italic     = 1 << 1,
    TextStyle_Underlined = 1 << 2,
};

struct text_grid_cell
{
    u32             CodePoint;
    linear_color    Background;
    linear_color    Foreground;
    text_style_bits Style;
};

struct text_grid
{
    u32             LineCount;
    u32             ColumnCount;
    text_grid_cell* Cells;
    v2u             CellSize;
    u32             LineGap;
};

internal b8 AllocateTextGrid(text_grid* Grid, u32 LineCount, u32 ColumnCount);

//============================================================================//
//------------------------------- RENDER GROUP -------------------------------//
//============================================================================//

enum draw_entry_type : u8
{
    DrawEntryType_Quad,
    DrawEntryType_Circle,
    DrawEntryType_TextGrid,
};

struct draw_entry_quad
{
    rect2s       Region;
    linear_color Color;
};

struct draw_entry_circle
{
    rect2s       Region;
    linear_color Color;
    v2f          MinUV;
    v2f          MaxUV;
    f32          InnerRadius;
};

struct draw_entry_text_grid
{
    text_grid Grid;
    font*     Font;
    v2s       Offset;
    rect2s    ClippingMask;
};

struct draw_entry
{
    draw_entry_type Type;
    union
    {
        draw_entry_quad      Quad;
        draw_entry_circle    Circle;
        draw_entry_text_grid TextGrid;
    };
};

struct render_group_page
{
    render_group_page* Prev;
    render_group_page* Next;
    draw_entry*        Entries;
    u32                Capacity;
    u32                Count;
};

struct render_group
{
    render_group_page* FirstPage;
    render_group_page* LastPage;
};

internal void PushQuad(render_group* Group,
                       arena* Arena,
                       rect2s Region,
                       linear_color Color);

internal void PushCircle(render_group* Group,
                         arena* Arena,
                         rect2s Region,
                         linear_color Color,
                         v2f MinUV,
                         v2f MaxUV,
                         f32 InnerRadius);

internal void PushTextGrid(render_group* Group,
                           arena* Arena,
                           text_grid* Grid,
                           font* Font,
                           v2s Offset, // Offset of the top-left corner.
                           rect2s ClippingMask);

internal void PushBorder(render_group* Group,
                         arena* Arena,
                         rect2s Region,
                         linear_color Color,
                         u32 BorderLeft,
                         u32 BorderRight,
                         u32 BorderBottom,
                         u32 BorderTop);

internal void PushRoundedQuad(render_group* Group,
                              arena* Arena,
                              rect2s Region,
                              linear_color Color,
                              u32 CornerRadius,
                              f32 CornerFallout);

internal void ExecuteRenderGroup(render_group* Group,
                                 bitmap* RenderTarget,
                                 rect2s ClippingMask);

#endif // RENDER_H
