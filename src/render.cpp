/*
 * Copyright (c) 2025-2026 Traian Avram. All rights reserved.
 * This file is part of my personal text editor and is distributed under the MIT license.
 */

//
// TEXT CURSOR:
//

struct TextCursor {
    Font* font;
    Rect2D visible_region;
    Vector2u cell_size;
    Vector2s cursor;
};

internal TextCursor
create_text_cursor(Font* font, Rect2D visible_region, Vector2s start_offset)
{
    TextCursor cursor = {};
    cursor.font = font;
    cursor.visible_region = visible_region;
    cursor.cell_size.x = font->advance_width;
    cursor.cell_size.y = font->line_spacing;
    cursor.cursor.x = visible_region.min.x + start_offset.x;
    cursor.cursor.y = visible_region.max.y - cursor.cell_size.y + start_offset.y + (font->descent);
    return cursor;
}

enum TextCursorOverflow : u8 {
    TextCursorOverflow_None,
    TextCursorOverflow_X,
    TextCursorOverflow_Y,
};

internal TextCursorOverflow
text_cursor_get_glyph_offset(TextCursor* cursor, Vector2s glyph_render_offset, Vector2s* out_offset)
{
    if (cursor->cursor.x >= cursor->visible_region.max.x ||
        cursor->cursor.x + cursor->cell_size.x <= cursor->visible_region.min.x)
    {
        // The current text cell is entirely out of the visible region along the Y-axis.
        ZERO_STRUCT_POINTER(out_offset);
        return TextCursorOverflow_X;
    }

    if (cursor->cursor.y >= cursor->visible_region.max.y ||
        cursor->cursor.y + cursor->cell_size.y <= cursor->visible_region.min.y)
    {
        // The current text cell is entirely out of the visible region along the Y-axis.
        ZERO_STRUCT_POINTER(out_offset);
        return TextCursorOverflow_Y;
    }

    *out_offset = cursor->cursor;
    out_offset->x += glyph_render_offset.x;
    out_offset->y += glyph_render_offset.y + (-cursor->font->descent);
    return TextCursorOverflow_None;
}

internal void
text_cursor_advance(TextCursor* cursor)
{
    cursor->cursor.x += cursor->cell_size.x;
}

internal void
text_cursor_next_line(TextCursor* cursor)
{
    cursor->cursor.x = cursor->visible_region.min.x;
    constant f32 LINE_SPACE_MULTIPLIER = 1.0F;
    cursor->cursor.y -= cursor->font->line_spacing * LINE_SPACE_MULTIPLIER;
}

//
// PRIMITIVE RENDERING:
//

internal void
render_quad_opaque_unoptimized(Rect2D rectangle, LinearColor color)
{
    s32 min_x = clamp(rectangle.min.x, 0, (s32)g_window_bitmap.size_x);
    s32 min_y = clamp(rectangle.min.y, 0, (s32)g_window_bitmap.size_y);
    s32 max_x = clamp(rectangle.max.x, 0, (s32)g_window_bitmap.size_x);
    s32 max_y = clamp(rectangle.max.y, 0, (s32)g_window_bitmap.size_y);

    usize row_byte_offset = (min_x * g_window_bitmap.bytes_per_pixel) + (min_y * g_window_bitmap.bytes_per_row);
    u32 packed_color = pack_bgra(color);

    for (s32 y = min_y; y < max_y; ++y) {
        u32* pixel = (u32*)(g_window_bitmap.pixels + row_byte_offset);
        for (s32 x = min_x; x < max_x; ++x) {
            *pixel = packed_color;
            ++pixel; // Advance to the next pixel in the row.
        }
        row_byte_offset += g_window_bitmap.bytes_per_row; // Advance to the next row.
    }
}

internal void
render_rectangle_opaque_unoptimized(Rect2D rectangle, u32 border_thickness, LinearColor color)
{
    // Ensure that the border doesn't overflow the rectangle shape.
    border_thickness = min(border_thickness, rect_size_x(rectangle) / 2 + 1);
    border_thickness = min(border_thickness, rect_size_y(rectangle) / 2 + 1);

    // Shortened aliases.
    auto r = rectangle;
    auto b = border_thickness;
    auto c = color;

    render_quad_opaque_unoptimized(rect(r.min.x,     r.min.y,     r.max.x - b, r.min.y + b), c); // Bottom
    render_quad_opaque_unoptimized(rect(r.max.x - b, r.min.y,     r.max.x,     r.max.y - b), c); // Right
    render_quad_opaque_unoptimized(rect(r.min.x + b, r.max.y - b, r.max.x,     r.max.y    ), c); // Top
    render_quad_opaque_unoptimized(rect(r.min.x,     r.min.y + b, r.min.x + b, r.max.y    ), c); // Left
}

internal void
render_glyph_bitmap_unoptimized(GrayscaleFontGlyph* glyph, Vector2s offset, Rect2D visible_region, LinearColor color)
{
    // Clamp the visible region inside the window bitmap bounding box.
    Rect2D window_bitmap_region = rect_offset_size(0, 0, g_window_bitmap.size_x, g_window_bitmap.size_y);
    visible_region = rect_intersect(visible_region, window_bitmap_region);

    // Clamp the destination offsets inside the visible region.
    s32 dst_min_x = clamp(offset.x,                      visible_region.min.x, visible_region.max.x);
    s32 dst_min_y = clamp(offset.y,                      visible_region.min.y, visible_region.max.y);
    s32 dst_max_x = clamp(offset.x + (s32)glyph->size.x, visible_region.min.x, visible_region.max.x);
    s32 dst_max_y = clamp(offset.y + (s32)glyph->size.y, visible_region.min.y, visible_region.max.y);

    s32 src_offset_x = dst_min_x - offset.x;
    s32 src_offset_y = dst_min_y - offset.y;

    usize dst_row_offset = (dst_min_x * g_window_bitmap.bytes_per_pixel) + (dst_min_y * g_window_bitmap.bytes_per_row);
    usize src_row_offset = (src_offset_x * glyph->bytes_per_pixel) + (src_offset_y * glyph->bytes_per_row);

    for (s32 y = dst_min_y; y < dst_max_y; ++y) {
        u32* dst_pixel = (u32*)(g_window_bitmap.pixels + dst_row_offset);
        u8*  src_sample = glyph->samples + src_row_offset;

        for (s32 x = dst_min_x; x < dst_max_x; ++x) {
            f32 sample_grayscale = *src_sample / 255.0F;

            LinearColor existing_color = unpack_bgra_to_linear_color(*dst_pixel);
            LinearColor new_color = lerp(existing_color, color, sample_grayscale);
            new_color.a = 0xFF;
            *dst_pixel = pack_bgra(new_color);

            dst_pixel++;
            src_sample++;
        }

        dst_row_offset += g_window_bitmap.bytes_per_row;
        src_row_offset += glyph->bytes_per_row;
    }
}

internal void
render_glyph_bitmap_unoptimized(SubpixelFontGlyph* glyph, Vector2s offset, Rect2D visible_region, LinearColor color)
{
    // Clamp the visible region inside the window bitmap bounding box.
    Rect2D window_bitmap_region = rect_offset_size(0, 0, g_window_bitmap.size_x, g_window_bitmap.size_y);
    visible_region = rect_intersect(visible_region, window_bitmap_region);

    // Clamp the destination offsets inside the visible region.
    s32 dst_min_x = clamp(offset.x,                      visible_region.min.x, visible_region.max.x);
    s32 dst_min_y = clamp(offset.y,                      visible_region.min.y, visible_region.max.y);
    s32 dst_max_x = clamp(offset.x + (s32)glyph->size.x, visible_region.min.x, visible_region.max.x);
    s32 dst_max_y = clamp(offset.y + (s32)glyph->size.y, visible_region.min.y, visible_region.max.y);

    s32 src_offset_x = dst_min_x - offset.x;
    s32 src_offset_y = dst_min_y - offset.y;

    usize dst_row_offset = (dst_min_x * g_window_bitmap.bytes_per_pixel) + (dst_min_y * g_window_bitmap.bytes_per_row);
    usize src_row_offset = (src_offset_x * glyph->bytes_per_pixel) + (src_offset_y * glyph->bytes_per_row);

    for (s32 y = dst_min_y; y < dst_max_y; ++y) {
        u32* dst_pixel = (u32*)(g_window_bitmap.pixels + dst_row_offset);
        u8*  src_samples = glyph->samples + src_row_offset;

        for (s32 x = dst_min_x; x < dst_max_x; ++x) {
            f32 alpha_r = (src_samples[2]) / 255.0F;
            f32 alpha_g = (src_samples[1]) / 255.0F;
            f32 alpha_b = (src_samples[0]) / 255.0F;

            LinearColor existing_color = unpack_bgra_to_linear_color(*dst_pixel);
            LinearColor new_color;
            new_color.b = lerp(existing_color.b, color.b, alpha_b);
            new_color.g = lerp(existing_color.g, color.g, alpha_g);
            new_color.r = lerp(existing_color.r, color.r, alpha_r);
            new_color.a = 0xFF;
            *dst_pixel = pack_bgra(new_color);

            dst_pixel += 1;
            src_samples += 3;
        }

        dst_row_offset += g_window_bitmap.bytes_per_row;
        src_row_offset += glyph->bytes_per_row;
    }
}

constant usize TITLEBAR_SIZE  = 50;
constant usize SCROLLBAR_SIZE = 15;
constant usize SPLITTER_SIZE  = 4;

internal void
render_editor_buffer(Rect2D buffer_region, EditorBuffer* buffer)
{
    render_rectangle_opaque_unoptimized(buffer_region, 2, linear_color(255, 0, 0));
}

internal void
render_editor_titlebar(Rect2D titlebar_region, u32 line_offset, u32 column_offset)
{
    render_rectangle_opaque_unoptimized(titlebar_region, 2, linear_color(0, 255, 0));
}

internal void
render_editor_scrollbar(Rect2D scrollbar_region, f32 offset, f32 height)
{
    render_rectangle_opaque_unoptimized(scrollbar_region, 2, linear_color(0, 0, 255));
}

internal void
render_editor_panel(Rect2D panel_region, EditorPanel* panel)
{
    Rect2D titlebar_region = {};
    titlebar_region.min = panel_region.min;
    titlebar_region.max.x = panel_region.max.x;
    titlebar_region.max.y = panel_region.min.y + TITLEBAR_SIZE;

    Rect2D buffer_region = {};
    Rect2D scrollbar_region = {};

    buffer_region.min.x = panel_region.min.x;
    buffer_region.max.x = panel_region.max.x;
    buffer_region.min.y = titlebar_region.max.y;
    buffer_region.max.y = panel_region.max.y;

    if (panel->is_scrollbar_visible) {
        buffer_region.max.x -= SCROLLBAR_SIZE;
        scrollbar_region.min.x = buffer_region.max.x;
        scrollbar_region.max.x = panel_region.max.x;
        scrollbar_region.min.y = buffer_region.min.y;
        scrollbar_region.max.y = panel_region.max.y;
    }

    if (!is_degenerated(buffer_region))
        render_editor_buffer(buffer_region, &panel->buffer);

    if (!is_degenerated(titlebar_region))
        render_editor_titlebar(titlebar_region, panel->caret.line_offset, panel->caret.column_offset);

    if (!is_degenerated(scrollbar_region))
        render_editor_scrollbar(scrollbar_region, panel->scrollbar_offset_percentage, panel->scrollbar_height_percentage);
}

internal void
render_editor_splitter(Rect2D splitter_region)
{
}

internal void
render_editor_frame(EditorState* state)
{
    Vector2u bitmap_size = {};
    bitmap_size.x = g_window_bitmap.size_x;
    bitmap_size.y = g_window_bitmap.size_y;
    
    // Don't render anything when the window is minimized.
    if (bitmap_size.x == 0 || bitmap_size.y == 0)
        return;

    if (state->is_splitscreen) {
        EditorPanel* l = &state->first_panel;
        EditorPanel* r = &state->second_panel;
        s32 available_size_x = bitmap_size.x - SPLITTER_SIZE;

        Rect2D l_panel_region = {};
        l_panel_region.min = v2s(0, 0);
        l_panel_region.max = v2s(available_size_x / 2, bitmap_size.y);

        Rect2D r_panel_region = {};
        r_panel_region.min = v2s(l_panel_region.max.x + SPLITTER_SIZE, 0);
        r_panel_region.max = v2s(bitmap_size.x, bitmap_size.y);

        if (!is_degenerated(l_panel_region) && !is_degenerated(r_panel_region)) {
            render_editor_panel(l_panel_region, &state->first_panel);
            render_editor_panel(r_panel_region, &state->second_panel);
        }

        Rect2D splitter_region = {};
        splitter_region.min.x = l_panel_region.max.x;
        splitter_region.min.y = 0;
        splitter_region.max.x = r_panel_region.min.x;
        splitter_region.max.y = bitmap_size.y;
        if (!is_degenerated(splitter_region))
            render_editor_splitter(splitter_region);
    }
}
