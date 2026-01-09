/*
 * Copyright (c) 2025-2026 Traian Avram. All rights reserved.
 * This file is part of my personal text editor and is distributed under the MIT license.
 */

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
    cursor.cell_size.y = font->ascent + (-font->descent);
    cursor.cursor.x = visible_region.min.x                      + start_offset.x;
    cursor.cursor.y = visible_region.max.y - cursor.cell_size.y + start_offset.y;
    return cursor;
}

struct TextCursorOverflow {
    bool vertical;
    bool horizontal;
};

internal TextCursorOverflow
get_overflow(TextCursor* cursor)
{
    TextCursorOverflow overflow;
    overflow.vertical   = false;
    overflow.horizontal = false;

    if (cursor->cursor.x >= cursor->visible_region.max.x ||
        cursor->cursor.x + cursor->cell_size.x <= cursor->visible_region.min.x)
    {
        // The current text cell is entirely out of the visible region along the Y-axis.
        overflow.horizontal = true;
    }

    if (cursor->cursor.y >= cursor->visible_region.max.y ||
        cursor->cursor.y + cursor->cell_size.y <= cursor->visible_region.min.y)
    {
        // The current text cell is entirely out of the visible region along the Y-axis.
        overflow.vertical = true;
    }

    return overflow;
}

internal Vector2s
get_glyph_offset(TextCursor* cursor, Vector2s glyph_render_offset)
{
    Vector2s glyph_offset;
    glyph_offset.x = cursor->cursor.x + glyph_render_offset.x;
    glyph_offset.y = cursor->cursor.y + glyph_render_offset.y + (-cursor->font->descent);
    return glyph_offset;
}

internal void
advance(TextCursor* cursor)
{
    cursor->cursor.x += cursor->cell_size.x;
}

internal void
next_line(TextCursor* cursor)
{
    cursor->cursor.x = cursor->visible_region.min.x;
    constant f32 LINE_SPACE_MULTIPLIER = 1.0F;
    cursor->cursor.y -= cursor->font->line_height * LINE_SPACE_MULTIPLIER;
}

//
// RENDER EDITOR FRAME:
//

constant usize TITLEBAR_SIZE  = 22;
constant usize SCROLLBAR_SIZE = 15;
constant usize SPLITTER_SIZE  = 8;

const LinearColor FOREGROUND_COLOR           = linear_color(205, 205, 165);
const LinearColor BACKGROUND_COLOR           = linear_color(35, 35, 35);
const LinearColor TITLEBAR_COLOR             = linear_color(189, 180, 98);
const LinearColor SPLITTER_COLOR             = linear_color(30, 30, 30);
const LinearColor SCROLLBAR_BACKGROUND_COLOR = linear_color(210, 210, 210);
const LinearColor SCROLLBAR_FOREGROUND_COLOR = linear_color(150, 150, 150);
const LinearColor SCROLLBAR_HOVERED_COLOR    = linear_color(140, 140, 140);
const LinearColor SCROLLBAR_IN_USE_COLOR     = linear_color(120, 120, 120);

//
// NOTE(Traian): The rendering routine for the editor buffer currently does too many things, such as determining
// the buffer offset based on the panel line and column offsets. Most of these things should be done by the update
// layer, and the rendering layer should only know what codepoints to render and what color (and effects) are they.
// The current arhictecture however is much simpler, and since we don't support any custom coloring for the text buffer
// it's fine... (8th January 2026)
//

internal void
render_editor_buffer(Rect2D buffer_region, EditorBuffer* buffer, u32 line_offset, u32 column_offset)
{
    TextCursor cursor = create_text_cursor(font_from_id(FontID_Text), buffer_region, v2s(0, 0));
    usize byte_offset = get_byte_offset_from_position(buffer, line_offset, column_offset);

    render_quad_opaque_unoptimized(buffer_region, BACKGROUND_COLOR);

    // @Incomplete: If we fail to decode a codepoint, we shouldn't stop rendering the buffer contents. We should
    // probably render a special glyph (to signal that the file is corrupted there) and carry on...
    Utf8Iterator buffer_iterator = utf8_iterator(buffer->data + byte_offset, buffer->size - byte_offset);
    while (is_valid(buffer_iterator)) {
        u32 codepoint = buffer_iterator.codepoint;
        advance(&buffer_iterator);

        TextCursorOverflow cursor_overflow = get_overflow(&cursor);
        if (cursor_overflow.vertical) break; // We ran out of real-estate on the Y-axis.

        if ('!' <= codepoint && codepoint <= '~') {
            if (!cursor_overflow.horizontal) {
                // auto* glyph = &cursor.font->ascii_subpixel_glyphs_freetype[codepoint - '!'];
                auto* glyph = &cursor.font->ascii_grayscale_glyphs_stb[codepoint - '!'];
                Vector2s glyph_offset = get_glyph_offset(&cursor, glyph->render_offset);
                render_glyph_bitmap_unoptimized(glyph, glyph_offset, buffer_region, FOREGROUND_COLOR);
            }
            advance(&cursor);
        } else if (codepoint == ' ') {
            advance(&cursor);
        } else if (codepoint == '\n') {
            next_line(&cursor);

            // @Cleanup: We should handle the CRLF/LF line ending dispute in a different way...
            if (is_valid(buffer_iterator) && buffer_iterator.codepoint == '\r')
                advance(&buffer_iterator);

            // @Cleanup: This should use some utility function at least that consumes the current row..
            u32 current_column_offset = 0;
            while (current_column_offset < column_offset && is_valid(buffer_iterator)) {
                // Let the next iteration of the outer while loop handle the new-line.
                if (buffer_iterator.codepoint == '\n')
                    break;

                ++current_column_offset;
                advance(&buffer_iterator);
            }

            // By the time we reached this point, we have either advance 'column_offset' codepoints (so we can
            // start rendering again), we have reached the end of the buffer (so nothing matters anymore), or
            // we have encountered a new-line character, so we let the next iteration handle it (since we haven't
            // consumed it).
        } else {
            // @Incomplete: Support more non-ASCII glyphs or at least display the raw hex values.
        }
    }
}

internal void
render_editor_titlebar(Rect2D titlebar_region, u32 line_offset, u32 column_offset)
{
    render_quad_opaque_unoptimized(titlebar_region, TITLEBAR_COLOR);
}

internal void
render_editor_scrollbar(Rect2D scrollbar_region, f32 offset, f32 height, ScrollbarState state)
{
    s32 scrollbar_height = rect_size_y(scrollbar_region) * height;
    s32 scrollbar_offset_y = lerp(scrollbar_region.max.y - scrollbar_height, scrollbar_region.min.y, offset);
    Rect2D scrollbar_box_region = {};
    scrollbar_box_region.min.x = scrollbar_region.min.x;
    scrollbar_box_region.min.y = scrollbar_offset_y;
    scrollbar_box_region.max.x = scrollbar_region.max.x;
    scrollbar_box_region.max.y = scrollbar_offset_y + scrollbar_height;

    LinearColor box_color = SCROLLBAR_FOREGROUND_COLOR;
    if (state == ScrollbarState_Visible) box_color = SCROLLBAR_FOREGROUND_COLOR;
    if (state == ScrollbarState_Hovered) box_color = SCROLLBAR_HOVERED_COLOR;
    if (state == ScrollbarState_InUse)   box_color = SCROLLBAR_IN_USE_COLOR;

    render_quad_opaque_unoptimized(scrollbar_region, SCROLLBAR_BACKGROUND_COLOR);
    render_quad_opaque_unoptimized(scrollbar_box_region, box_color);
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

    if (panel->scrollbar_state != ScrollbarState_Hidden) {
        buffer_region.max.x -= SCROLLBAR_SIZE;
        scrollbar_region.min.x = buffer_region.max.x;
        scrollbar_region.max.x = panel_region.max.x;
        scrollbar_region.min.y = buffer_region.min.y;
        scrollbar_region.max.y = panel_region.max.y;
    }

    if (!is_degenerated(buffer_region))
        render_editor_buffer(buffer_region, &panel->buffer, panel->caret.line_offset, panel->caret.column_offset);

    if (!is_degenerated(titlebar_region))
        render_editor_titlebar(titlebar_region, panel->caret.line_offset, panel->caret.column_offset);

    if (!is_degenerated(scrollbar_region)) {
        render_editor_scrollbar(scrollbar_region, panel->scrollbar_offset_percentage,
                                panel->scrollbar_height_percentage, panel->scrollbar_state);
    }
}

internal void
render_editor_splitter(Rect2D splitter_region)
{
    // @Cleanup: It would be really neat to actually use 'rect_intersect' in order to compute this regions!
    Rect2D titlebar_intersection_region = {};
    titlebar_intersection_region.min = splitter_region.min;
    titlebar_intersection_region.max.x = splitter_region.max.x;
    titlebar_intersection_region.max.y = splitter_region.min.y + TITLEBAR_SIZE;
    render_quad_opaque_unoptimized(titlebar_intersection_region, TITLEBAR_COLOR);

    // @Cleanup: It would be really neat to actually use 'rect_intersect' in order to compute this regions!
    Rect2D buffer_intersection_region = {};
    buffer_intersection_region.min.x = splitter_region.min.x;
    buffer_intersection_region.min.y = titlebar_intersection_region.max.y;
    buffer_intersection_region.max = splitter_region.max;
    render_quad_opaque_unoptimized(buffer_intersection_region, SPLITTER_COLOR);
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
        Rect2D r_panel_region = {};
        Rect2D l_panel_region = {};
        s32 splitter_size = SPLITTER_SIZE;
        if (state->first_panel.scrollbar_state != ScrollbarState_Hidden)
            splitter_size = 0; // Hide the splitter when the scrollbar is already there.
        s32 available_size_x = bitmap_size.x - splitter_size;

        l_panel_region.min = v2s(0, 0);
        l_panel_region.max = v2s(available_size_x / 2, bitmap_size.y);
        r_panel_region.min = v2s(l_panel_region.max.x + splitter_size, 0);
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
    } else {
        Rect2D panel_region = rect_offset_size(0, 0, bitmap_size.x, bitmap_size.y);
        if (!is_degenerated(panel_region))
            render_editor_panel(panel_region, &state->first_panel);
    }
}
