/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#include <Core/Assertion.h>
#include <Core/Memory/MemoryOperations.h>
#include <Graphics/Font.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <Graphics/stb_truetype.h>

static void flip_vertical_glyph_bitmap(ReadWriteBytes data, u32 width, u32 height)
{
    ReadWriteBytes row_iterator_a = data;
    ReadWriteBytes row_iterator_b = data + ((usize)width * (usize)(height - 1));

    for (u32 y_offset = 0; y_offset < height / 2; ++y_offset) {
        swap_memory(row_iterator_a, row_iterator_b, width);
        row_iterator_a += width;
        row_iterator_b -= width;
    }
}

void font_create_from_memory(Font* font, LinearArena* arena, float font_height, ReadonlyByteSpan ttf_memory_buffer, GraphicsContext graphics_context)
{
    font->height = font_height;

    stbtt_fontinfo font_info = {};
    stbtt_InitFont(&font_info, ttf_memory_buffer.bytes, stbtt_GetFontOffsetForIndex(ttf_memory_buffer.bytes, 0));
    const float scale = stbtt_ScaleForPixelHeight(&font_info, font->height);

    int advance;
    stbtt_GetCodepointHMetrics(&font_info, 'X', &advance, nullptr);
    font->advance = (u32)(advance * scale);

    int ascent;
    int descent;
    int line_gap;
    stbtt_GetFontVMetrics(&font_info, &ascent, &descent, &line_gap);
    font->ascent = (u32)(ascent * scale);
    font->descent = (u32)(-descent * scale);
    font->line_gap = (u32)(line_gap * scale);

    for (u32 ascii_cache_index = 0; ascii_cache_index < FONT_ASCII_GLYPH_CACHE_CHARACTER_COUNT; ++ascii_cache_index) {
        const u32 codepoint = FONT_ASCII_GLYPH_CACHE_FIRST_CHARACTER + ascii_cache_index;
        FontGlyph* glyph = font->ascii_glyph_cache.glyphs + ascii_cache_index;

        int width;
        int height;
        int offset_x;
        int offset_y;
        ReadWriteBytes glyph_bitmap_data = stbtt_GetCodepointBitmap(&font_info, 0, scale, codepoint, &width, &height, &offset_x, &offset_y);
        glyph->offset_x = offset_x;
        // NOTE: The stbtt generated bitmap is Y-down (top-left origin).
        glyph->offset_y = -height - offset_y;

        flip_vertical_glyph_bitmap(glyph_bitmap_data, width, height);
        glyph->bitmap = graphics_context_allocate_bitmap(graphics_context, arena, width, height, GRAPHICS_BITMAP_USAGE_FONT);
        graphics_bitmap_set_data(graphics_context, glyph->bitmap, readonly_byte_span(glyph_bitmap_data, (usize)width * (usize)height));

        stbtt_FreeBitmap(glyph_bitmap_data, nullptr);
    }
}

void font_destroy(Font* font, GraphicsContext graphics_context)
{
    for (u32 ascii_cache_index = 0; ascii_cache_index < FONT_ASCII_GLYPH_CACHE_CHARACTER_COUNT; ++ascii_cache_index) {
        FontGlyph* glyph = font->ascii_glyph_cache.glyphs + ascii_cache_index;
        graphics_context_release_bitmap(graphics_context, &glyph->bitmap);
        glyph->offset_x = 0;
        glyph->offset_y = 0;
    }

    *font = {};
}

const FontGlyph* font_get_glyph(const Font* font, u32 unicode_codepoint)
{
    if (FONT_ASCII_GLYPH_CACHE_FIRST_CHARACTER <= unicode_codepoint && unicode_codepoint <= FONT_ASCII_GLYPH_CACHE_LAST_CHARACTER) {
        const u32 cache_index = unicode_codepoint - FONT_ASCII_GLYPH_CACHE_FIRST_CHARACTER;
        return font->ascii_glyph_cache.glyphs + cache_index;
    }

    // TODO: Add support for all Unicode codepoints.
    VERIFY_NOT_REACHED;
    return nullptr;
}
