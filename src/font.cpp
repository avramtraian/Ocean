/*
 * Copyright (c) 2025-2026 Traian Avram. All rights reserved.
 * This file is part of my personal text editor and is distributed under the MIT license.
 */

struct GrayscaleFontGlyph {
    constant usize bytes_per_pixel = 1;
    u8* samples;
    usize bytes_per_row;
    Vector2u size;
    Vector2s render_offset;
    s32 advance_width;
};

struct SubpixelFontGlyph {
    constant usize bytes_per_pixel = 3;
    u8* samples;
    usize bytes_per_row;
    Vector2u size;
    Vector2s render_offset;
    s32 advance_width;    
};

struct Font {
    u32 pixel_height;
    s32 advance_width;
    s32 ascent;
    s32 descent;
    s32 line_height;
    s32 line_gap;

    constant usize ascii_glyph_count = '~' - '!' + 1;
    GrayscaleFontGlyph ascii_grayscale_glyphs_stb      [ascii_glyph_count];
    GrayscaleFontGlyph ascii_grayscale_glyphs_freetype [ascii_glyph_count];
    SubpixelFontGlyph  ascii_subpixel_glyphs_freetype  [ascii_glyph_count];
};

internal void
load_font_stb(Font* font, char* font_file_name, u32 pixel_height, MemoryArena* arena)
{
    // NOTE(Traian): Failing to load a font crashes the program since the rest of the codebase
    // assumes that every font is valid and loaded. (4th January 2025)

    // @Incomplete: Uncomment the following line. We currently don't do this because we call both
    // 'load_font_stb' and 'load_font_freetype' for testing purposes, but in the future
    // we would obviously rasterize the fonts once... do the same thing in 'load_font_freetype'.
    // ZERO_STRUCT_POINTER(font);

    OSReadFileResult file = os_read_entire_file(font_file_name);
    ASSERT(file.is_valid);

    stbtt_fontinfo font_info = {};
    bool load_font_success = stbtt_InitFont(&font_info, file.data, stbtt_GetFontOffsetForIndex(file.data, 0));
    ASSERT(load_font_success);

    font->pixel_height = pixel_height;
    f32 scale_for_height = stbtt_ScaleForPixelHeight(&font_info, font->pixel_height);

    int ascent, descent, line_gap, font_advance_width;
    stbtt_GetCodepointHMetrics(&font_info, 'X', &font_advance_width, NULL);
    // NOTE(Traian): Since we only support monospaced fonts this value should be the same for all
    // codepoints supported by the font. For convenience we store the value directly in the font.
    stbtt_GetFontVMetrics(&font_info, &ascent, &descent, &line_gap);
    font->ascent        = ascent   * scale_for_height;
    font->descent       = descent  * scale_for_height;
    font->line_gap      = line_gap * scale_for_height;
    font->advance_width = font_advance_width * scale_for_height;
    font->line_height   = font->ascent + (-font->descent) + font->line_gap;

    for (int ascii_codepoint = '!'; ascii_codepoint <= '~'; ++ascii_codepoint) {
        GrayscaleFontGlyph* glyph = &font->ascii_grayscale_glyphs_stb[ascii_codepoint - '!'];
        ZERO_STRUCT_POINTER(glyph);

        // Query horizontal metrics.
        int advance_width, left_side_bearing;
        stbtt_GetCodepointHMetrics(&font_info, ascii_codepoint, &advance_width, &left_side_bearing);
        advance_width     *= scale_for_height;
        left_side_bearing *= scale_for_height;

        // Query the bitmap bounding box.
        int min_x, min_y, max_x, max_y;
        stbtt_GetCodepointBitmapBox(&font_info, ascii_codepoint, scale_for_height, scale_for_height,
                                  &min_x, &min_y, &max_x, &max_y);
        glyph->size.x = max_x - min_x;
        glyph->size.y = max_y - min_y;
        glyph->bytes_per_row = glyph->size.x * glyph->bytes_per_pixel;
        glyph->samples = PUSH_ARRAY(arena, u8, glyph->size.y * glyph->bytes_per_row);
        
        glyph->render_offset.x = left_side_bearing;
        glyph->render_offset.y = -(glyph->size.y + min_y); // stb_truetype uses Y-down while we use Y-up...
        ASSERT(advance_width == font->advance_width);

        // Rasterize the glyph bitmap.
        stbtt_MakeCodepointBitmap(&font_info, glyph->samples,
                                  glyph->size.x, glyph->size.y, glyph->size.x,
                                  scale_for_height, scale_for_height, ascii_codepoint);

        // NOTE(Traian): The stb_truetype generates the bitmaps using the Y-down convention (and thus the
        // origin is the top-left corner), but we use the Y-up convention everywhere else in the codebase.
        // Unlike stb_image, there is no API you can use to tell the library to automatically flip the
        // bitmap, so we must do it manually. (3rd January 2025)
        usize dst_row_offset = 0;
        usize src_row_offset = (glyph->size.y - 1) * glyph->bytes_per_row;
        for (u32 y = 0; y < glyph->size.y / 2; ++y) {
            u8* dst = glyph->samples + dst_row_offset;
            u8* src = glyph->samples + src_row_offset;
            for (u32 x = 0; x < glyph->bytes_per_row; ++x) {
                u8 temp = *dst;
                *dst = *src;
                *src = temp;
                ++dst;
                ++src;
            }

            dst_row_offset += glyph->bytes_per_row;
            src_row_offset -= glyph->bytes_per_row;
        }
    }

    os_free_read_file_result(file);
}

internal void
load_font_freetype(Font* font, char* font_file_name, u32 pixel_height, MemoryArena* arena)
{
    // NOTE(Traian): Failing to load a font crashes the program since the rest of the codebase
    // assumes that every font is valid and loaded. (4th January 2025)

    // @Incomplete: Uncomment the following line. We currently don't do this because we call both
    // 'load_font_stb' and 'load_font_freetype' for testing purposes, but in the future
    // we would obviously rasterize the fonts once... do the same thing in 'load_font_stb'.
    // ZERO_STRUCT_POINTER(font);

    FT_Library library;
    FT_Error initialize_library_result = FT_Init_FreeType(&library);
    ASSERT(initialize_library_result == 0);
    
    // According to the freetype documentation, this function might fail depending on what library
    // we are actually linking against?
    FT_Error set_lcd_filter_result = FT_Library_SetLcdFilter(library, FT_LCD_FILTER_DEFAULT);
    // ASSERT(set_lcd_filter_result == 0); // @Incomplete!

    FT_Face face;
    FT_Error new_face_result = FT_New_Face(library, font_file_name, 0, &face);
    ASSERT(new_face_result == 0);

    FT_Error set_pixel_size_result = FT_Set_Pixel_Sizes(face, 0, pixel_height);
    ASSERT(set_pixel_size_result == 0);

    font->ascent      = face->size->metrics.ascender  / 64;
    font->descent     = face->size->metrics.descender / 64;
    font->line_height = face->size->metrics.height    / 64;
    font->line_gap    = font->line_height - (font->ascent + (-font->descent));

    for (int ascii_codepoint = '!'; ascii_codepoint <= '~'; ++ascii_codepoint) {
        FT_Error load_char_result = FT_Load_Char(face, ascii_codepoint, FT_LOAD_FORCE_AUTOHINT | FT_LOAD_RENDER | FT_LOAD_TARGET_LCD);
        ASSERT(load_char_result == 0);

        SubpixelFontGlyph* glyph = &font->ascii_subpixel_glyphs_freetype[ascii_codepoint - '!'];
        ZERO_STRUCT_POINTER(glyph);

        glyph->size.x = face->glyph->bitmap.width / glyph->bytes_per_pixel;
        glyph->size.y = face->glyph->bitmap.rows;

        glyph->render_offset.x = face->glyph->bitmap_left;
        glyph->render_offset.y = -(face->glyph->bitmap.rows - face->glyph->bitmap_top);
        glyph->advance_width = face->glyph->advance.x / 64;

        // NOTE(Traian): Since we only support monospaced fonts, this value should be the same for all
        // codepoints. However, there are some fonts (such as 'Droid Sans Mono') that seem to don't to this?
        font->advance_width = glyph->advance_width;

        glyph->samples = PUSH_ARRAY(arena, u8, face->glyph->bitmap.rows * face->glyph->bitmap.pitch);
        glyph->bytes_per_row = face->glyph->bitmap.pitch;

        usize dst_byte_offset = 0;
        usize src_byte_offset = (glyph->size.y - 1) * glyph->bytes_per_row;
        
        // @Incomplete: If the ghyph requries flipping along the Y-axis, then flip it! Otherwise, this has no
        // point. Just copy the bytes directly using a single 'copy_memory' call.
        for (u32 y = 0; y < glyph->size.y; ++y) {
            copy_memory(glyph->samples + dst_byte_offset,
                        face->glyph->bitmap.buffer + src_byte_offset,
                        glyph->bytes_per_row);

            dst_byte_offset += glyph->bytes_per_row;
            src_byte_offset -= glyph->bytes_per_row;
        }
    }

    FT_Done_Face(face);
    FT_Done_FreeType(library);
}

struct GlobalFonts {
    Font text;
    Font ui;
    Font ui_small;
    Font ui_big;
};

internal GlobalFonts g_fonts;

enum FontID : u8 {
    FontID_Text,
    FontID_UI,
    FontID_UISmall,
    FontID_UIBig,
    FontID_MaxEnumCount,
};

inline Font*
font_from_id(FontID font_id)
{
    switch (font_id) {
      case FontID_Text:    return &g_fonts.text;
      case FontID_UI:      return &g_fonts.ui;
      case FontID_UISmall: return &g_fonts.ui_small;
      case FontID_UIBig:   return &g_fonts.ui_big;
      default:             return NULL; // ID is invalid.
    }
}

struct GlobalFontsDescription {
    // @Robustness: Use custom String type instead of null-terminated.
    char* text_name;
    char* ui_name;
    char* ui_small_name;
    char* ui_big_name;
    u32 text_pixel_height;
    u32 ui_pixel_height;
    u32 ui_small_pixel_height;
    u32 ui_big_pixel_height;
};

internal void
reload_global_fonts(GlobalFontsDescription* description)
{
    MemoryArena* arena = g_arenas.fonts;
    reset_memory_arena(arena);

    load_font_stb(&g_fonts.text,     description->text_name,     description->text_pixel_height,     arena);
    load_font_stb(&g_fonts.ui,       description->ui_name,       description->ui_pixel_height,       arena);
    load_font_stb(&g_fonts.ui_small, description->ui_small_name, description->ui_small_pixel_height, arena);
    load_font_stb(&g_fonts.ui_big,   description->ui_big_name,   description->ui_big_pixel_height,   arena);

    // load_font_freetype(&g_fonts.text,     description->text_name,     description->text_pixel_height,     arena);
    // load_font_freetype(&g_fonts.ui,       description->ui_name,       description->ui_pixel_height,       arena);
    // load_font_freetype(&g_fonts.ui_small, description->ui_small_name, description->ui_small_pixel_height, arena);
    // load_font_freetype(&g_fonts.ui_big,   description->ui_big_name,   description->ui_big_pixel_height,   arena);
}
