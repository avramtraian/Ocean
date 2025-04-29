/**
 * Copyright (c) 2025 Traian Avram. All rights reserved.
 * This source file is part of the Ocean text editor.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#pragma once

#include "core.h"
#include "graphics.h"

function void draw_clear_bitmap(Bitmap *bitmap, LinearColor clear_color);
function void draw_quad(Bitmap *bitmap, s32 offset_x, s32 offset_y, u32 size_x, u32 size_y, LinearColor color);
function void draw_rectangle(Bitmap *bitmap, s32 offset_x, s32 offset_y, u32 size_x, u32 size_y, u32 thickness, LinearColor color);
function void draw_rectangle_containing(Bitmap *bitmap, s32 offset_x, s32 offset_y, u32 size_x, u32 size_y, u32 thickness, LinearColor color);

function void draw_glyph_bitmap(
    Bitmap *bitmap, Bitmap *glyph_bitmap, s32 offset_x, s32 offset_y, LinearColor color,
    u32 viewport_offset_x, u32 viewport_offset_y, u32 viewport_size_x, u32 viewport_size_y);

function void
draw_tiled_text_buffer(Bitmap *bitmap, TiledTextBuffer *buffer, Font *font);
