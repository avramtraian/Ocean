/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#pragma once

#include <Core/Memory/Arena.h>
#include <Graphics/GraphicsContext.h>

struct FontGlyph {
    GraphicsBitmap bitmap;
    u32 offset_x;
    u32 offset_y;
};

#define FONT_ASCII_GLYPH_CACHE_FIRST_CHARACTER ((u32)'!')
#define FONT_ASCII_GLYPH_CACHE_LAST_CHARACTER  ((u32)'~')
#define FONT_ASCII_GLYPH_CACHE_CHARACTER_COUNT (FONT_ASCII_GLYPH_CACHE_LAST_CHARACTER - FONT_ASCII_GLYPH_CACHE_FIRST_CHARACTER + 1)

struct FontASCIIGlyphCache {
    // NOTE: Very fast access cache for ASCII characters, as they represent the majority of the characters that
    //       are encountered by a text editor used for coding.
    FontGlyph glyphs[FONT_ASCII_GLYPH_CACHE_CHARACTER_COUNT];
};

struct Font {
    float height;
    u32 advance;
    u32 ascent;
    u32 descent;
    u32 line_gap;
    // NOTE: Caches for common character glyphs.
    FontASCIIGlyphCache ascii_glyph_cache;
};

void font_create_from_memory(Font* font, LinearArena* arena, float font_height, ReadonlyByteSpan ttf_memory_buffer, GraphicsContext graphics_context);
void font_destroy(Font* font, GraphicsContext graphics_context);

const FontGlyph* font_get_glyph(const Font* font, u32 unicode_codepoint);
