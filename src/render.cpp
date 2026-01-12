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
    Vector2s start_offset;
    Vector2s current_cell_offset;
};

internal TextCursor
create_text_cursor(Font* font, Rect2D visible_region, Vector2s start_offset)
{
    TextCursor cursor = {};
    cursor.font = font;
    cursor.visible_region = visible_region;
    cursor.start_offset = start_offset;
    cursor.current_cell_offset.x = visible_region.min.x                           + start_offset.x;
    cursor.current_cell_offset.y = visible_region.max.y - font->glyph_cell_size.y + start_offset.y;
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

    if (cursor->current_cell_offset.x >= cursor->visible_region.max.x ||
        cursor->current_cell_offset.x + cursor->font->glyph_cell_size.x <= cursor->visible_region.min.x)
    {
        // The current text cell is entirely out of the visible region along the Y-axis.
        overflow.horizontal = true;
    }

    if (cursor->current_cell_offset.y >= cursor->visible_region.max.y ||
        cursor->current_cell_offset.y + cursor->font->glyph_cell_size.y <= cursor->visible_region.min.y)
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
    glyph_offset.x = cursor->current_cell_offset.x + glyph_render_offset.x;
    glyph_offset.y = cursor->current_cell_offset.y + glyph_render_offset.y + (-cursor->font->descent);
    return glyph_offset;
}

internal void
advance(TextCursor* cursor, u32 cell_count)
{
    cursor->current_cell_offset.x += cell_count * cursor->font->glyph_cell_size.x;
}

internal void
next_line(TextCursor* cursor)
{
    cursor->current_cell_offset.x = cursor->visible_region.min.x + cursor->start_offset.x;
    cursor->current_cell_offset.y -= cursor->font->line_height;
}

//
// RENDER EDITOR FRAME:
//

constant f32 CURSOR_SIZE_PERCENTAGE_X = 0.2F;
constant f32 CURSOR_SIZE_PERCENTAGE_Y = 1.3F;

const LinearColor FOREGROUND_COLOR           = linear_color(205, 205, 165);
const LinearColor BACKGROUND_COLOR           = linear_color(35, 35, 35);
const LinearColor BACKGROUND_SELECTED_COLOR  = linear_color(15, 30, 200);
const LinearColor TITLEBAR_BACKGROUND_COLOR  = linear_color(189, 180, 98);
const LinearColor TITLEBAR_FOREGROUND_COLOR  = linear_color(25, 25, 25);
const LinearColor SPLITTER_COLOR             = linear_color(30, 30, 30);
const LinearColor SCROLLBAR_BACKGROUND_COLOR = linear_color(210, 210, 210);
const LinearColor SCROLLBAR_FOREGROUND_COLOR = linear_color(150, 150, 150);
const LinearColor SCROLLBAR_HOVERED_COLOR    = linear_color(140, 140, 140);
const LinearColor SCROLLBAR_IN_USE_COLOR     = linear_color(120, 120, 120);
const LinearColor CURSOR_COLOR               = linear_color(220, 220, 220);

//
// NOTE(Traian): The rendering routine for the editor buffer currently does too many things, such as determining
// the buffer offset based on the panel line and column offsets. Most of these things should be done by the update
// layer, and the rendering layer should only know what codepoints to render and what color (and effects) are they.
// The current arhictecture however is much simpler, and since we don't support any custom coloring for the text buffer
// it's fine... (8th January 2026)
//

internal void
draw_editor_buffer(Rect2D buffer_region, EditorBuffer* buffer,
                     usize selection_start_offset, usize selection_end_offset,
                     u32 line_offset, u32 column_offset)
{
    if (is_degenerated(buffer_region))
        return;

    render_quad_opaque_unoptimized(buffer_region, BACKGROUND_COLOR);

    if (selection_start_offset > selection_end_offset) {
        usize temp = selection_start_offset;
        selection_start_offset = selection_end_offset;
        selection_end_offset = temp;
    }

    Font* font = font_from_id(FontID_Text);

    Vector2s cursor_start_offset = v2s(-column_offset * font->glyph_cell_size.x, 0);
    TextCursor cursor = create_text_cursor(font, buffer_region, cursor_start_offset);

    usize line_byte_offset = get_line_byte_offset(buffer, line_offset);
    // @Incomplete: If we fail to decode a codepoint, we shouldn't stop rendering the buffer contents. We should
    // probably render a special glyph (to signal that the file is corrupted there) and carry on...
    Utf8Iterator buffer_iterator = utf8_iterator(buffer->data + line_byte_offset, buffer->size - line_byte_offset);
    while (is_valid(buffer_iterator)) {
        u32 codepoint = buffer_iterator.codepoint;
        usize buffer_byte_offset = line_byte_offset + buffer_iterator.offset;
        advance(&buffer_iterator);

        TextCursorOverflow cursor_overflow = get_overflow(&cursor);
        if (cursor_overflow.vertical) break; // We ran out of real-estate on the Y-axis.

        if ((selection_start_offset <= buffer_byte_offset) &&
            (buffer_byte_offset < selection_end_offset))
        {
            // We are rendering character that is inside the cursor selection.
            Rect2D cell_region = rect_offset_size(cursor.current_cell_offset, to_v2u(font->glyph_cell_size));
            cell_region = rect_intersect(cell_region, buffer_region);

            if (!is_degenerated(cell_region)) {
                render_quad_opaque_unoptimized(cell_region, BACKGROUND_SELECTED_COLOR);
            }
        }

        // @Cleanup: We should handle the CRLF/LF line ending dispute in a different way...
        if (codepoint == '\r') {
            if (is_valid(buffer_iterator) && buffer_iterator.codepoint == '\n') {
                codepoint = '\n';
                advance(&buffer_iterator);
            }
        }

        if ('!' <= codepoint && codepoint <= '~') {
            if (!cursor_overflow.horizontal) {
                // auto* glyph = &cursor.font->ascii_subpixel_glyphs_freetype[codepoint - '!'];
                auto* glyph = &font->ascii_grayscale_glyphs_stb[codepoint - '!'];
                Vector2s glyph_offset = get_glyph_offset(&cursor, glyph->render_offset);
                render_glyph_bitmap_unoptimized(glyph, glyph_offset, buffer_region, FOREGROUND_COLOR);
            }
            advance(&cursor, 1);
        } else if (codepoint == ' ') {
            advance(&cursor, 1);
        } else if (codepoint == '\n') {
            next_line(&cursor);
        } else {
            // @Incomplete: Support more non-ASCII glyphs or at least display the raw hex values.
        }
    }
}

internal void
draw_editor_cursor(Rect2D buffer_region, EditorPanel* panel)
{
    if (is_degenerated(buffer_region))
        return;

    Font* text_font = font_from_id(FontID_Text);

    // Relative to the buffer top-left corner.
    BufferPosition cursor_position = get_position_from_byte_offset(&panel->buffer, panel->cursor.state.byte_offset);
    s32 line_index   = (s32)cursor_position.line_offset   - (s32)panel->first_line_offset;
    s32 column_index = (s32)cursor_position.column_offset - (s32)panel->first_column_offset;

    if (line_index >= 0 && column_index >= 0) {
        Vector2s cursor_offset;
        cursor_offset.x = buffer_region.min.x + (column_index * text_font->glyph_cell_size.x);
        cursor_offset.y = buffer_region.max.y - text_font->glyph_cell_size.y - (line_index * text_font->line_height);

        Vector2u cursor_size;
        cursor_size.x = text_font->glyph_cell_size.x * CURSOR_SIZE_PERCENTAGE_X;
        cursor_size.y = text_font->glyph_cell_size.y * CURSOR_SIZE_PERCENTAGE_Y;

        // Center the cursor horizontally.
        cursor_offset.y += (text_font->glyph_cell_size.y - (s32)cursor_size.y) / 2;

        Rect2D cursor_region = rect_offset_size(cursor_offset, cursor_size);
        cursor_region = rect_intersect(cursor_region, buffer_region);
        if (!is_degenerated(cursor_region))
            render_quad_opaque_unoptimized(cursor_region, CURSOR_COLOR);
    }
}

internal void
draw_editor_titlebar(Rect2D titlebar_region, String title, u32 line_offset, u32 column_offset,
                       u32 max_line_offset, u32 max_column_offset)
{
    if (is_degenerated(titlebar_region))
        return;

    render_quad_opaque_unoptimized(titlebar_region, TITLEBAR_BACKGROUND_COLOR);
    Font* titlebar_font = font_from_id(FontID_UI);

    // Offsets of the cursor position start at 0, but the editor displays them starting from 1.
    String line       = string_from_number((u64)line_offset + 1);
    String column     = string_from_number((u64)column_offset + 1);
    String max_line   = string_from_number((u64)max_line_offset + 1);
    String max_column = string_from_number((u64)max_column_offset + 1);

    u32 line_space_padding = max_line.size - line.size;
    u32 column_space_padding = max_column.size - column.size;

    u32 line_codepoint_count   = 2 + line.size + line_space_padding;
    u32 column_codepoint_count = 2 + column.size + column_space_padding;
    u32 title_codepoint_count  = utf8_get_codepoint_count(title.data, title.size);

    // NOTE(Traian): @Cleanup: This routine is very convoluted, mostly due to a bad standard library.
    // The purpose of this code is quite simple, find what information we can fit on the titlebar, construct
    // that information message, and render it. The implementation of the first step is simple to understand,
    // but would require a complete rewrite if we want to display more information other than title, line and
    // column numbers. The second step should however be much concise!!!! (9th January 2026)

    //
    // Find what information we can fit on the titlebar:
    //

    u32 max_length = rect_size_x(titlebar_region) / titlebar_font->advance_width;
    constant u32 MIN_TITLE_DISPLAY_COUNT = 8; // Even if we have available space, unless the title is actually shorter, we will never display less codepoints than this value from the title.
    u32 min_title_display_count = min(title_codepoint_count, MIN_TITLE_DISPLAY_COUNT);

    u32 line_and_column_count           = line_codepoint_count + 1 + column_codepoint_count;
    u32 title_and_line_and_column_count = min_title_display_count + 1 + line_and_column_count;

    bool display_title         = false;
    bool display_line_number   = false;
    bool display_column_number = false;

    if (max_length >= title_and_line_and_column_count) {
        display_title         = true;
        display_line_number   = true;
        display_column_number = true;
    } else if (max_length >= line_and_column_count) {
        display_line_number   = true;
        display_column_number = true;
    } else if (max_length >= line_codepoint_count) {
        display_line_number = true;
    } else {
        // We don't have enough space to display anything.

        // NOTE(Traian): @Incomplete: At this point, maybe we should hide the titlebar completely? Working with
        // dimensions this small however means that the user doesn't really use the editor in this moment, so the
        // complixty of the program we introduce by this "feature" will not be used very often! (9th January 2026)
    }

    //
    // Construct the information message that will be rendered on the titlebar:
    //

    String titlebar_text = push_string_frame(4 * max_length); // Assume the worst case, when all codepoints are encoded as 4-byte sequences.
    titlebar_text.size = 0; // We can freely change this value, as the memory is allocated from an arena.

    if (display_title) {
        u32 codepoint_count = min(max_length - (1 + line_and_column_count), title_codepoint_count);
        u32 whitespace_count = max_length - (codepoint_count + line_and_column_count);

        u32 current_codepoint_index = 0;
        Utf8Iterator title_iterator = utf8_iterator(title.data, title.size);
        while (current_codepoint_index < codepoint_count) {
            ASSERT(is_valid(title_iterator));
            copy_memory(titlebar_text.data + titlebar_text.size,
                        title.data + title_iterator.offset,
                        title_iterator.byte_width);
            titlebar_text.size += title_iterator.byte_width;
            ++current_codepoint_index;
            advance(&title_iterator);
        }

        ASSERT(whitespace_count > 0);
        for (u32 i = 0; i < whitespace_count; ++i) {
            titlebar_text.data[titlebar_text.size] = ' ';
            titlebar_text.size++;
        }
    }

    if (display_line_number) {
        String prefix = STRING_LIT("L#");
        copy_memory(titlebar_text.data + titlebar_text.size, prefix.data, prefix.size);
        titlebar_text.size += prefix.size;

        copy_memory(titlebar_text.data + titlebar_text.size, line.data, line.size);
        titlebar_text.size += line.size;

        for (u32 i = 0; i < line_space_padding; ++i) {
            titlebar_text.data[titlebar_text.size] = ' ';
            titlebar_text.size++;
        }
    }

    if (display_column_number) {
        String prefix = STRING_LIT(" C#");
        copy_memory(titlebar_text.data + titlebar_text.size, prefix.data, prefix.size);
        titlebar_text.size += prefix.size;

        copy_memory(titlebar_text.data + titlebar_text.size, column.data, column.size);
        titlebar_text.size += column.size;

        for (u32 i = 0; i < column_space_padding; ++i) {
            titlebar_text.data[titlebar_text.size] = ' ';
            titlebar_text.size++;
        }
    }

    Vector2u text_line_size = get_text_line_size(titlebar_font, titlebar_text);
    Rect2D text_region = {};
    text_region.min.x = titlebar_region.min.x + (((s32)rect_size_x(titlebar_region) - text_line_size.x) / 2);
    text_region.min.y = titlebar_region.min.y + (((s32)rect_size_y(titlebar_region) - text_line_size.y) / 2);
    text_region.max.x = text_region.min.x + text_line_size.x;
    text_region.max.y = text_region.min.y + text_line_size.y;

    if (!is_degenerated(text_region)) {
        TextCursor cursor = create_text_cursor(titlebar_font, text_region, v2s(0, 0));
        for (Utf8Iterator iterator = utf8_iterator(titlebar_text);
             is_valid(iterator);
             advance(&iterator))
        {
            u32 codepoint = iterator.codepoint;
            if ('!' <= codepoint && codepoint <= '~') {
                auto* glyph = &titlebar_font->ascii_grayscale_glyphs_stb[codepoint - '!'];
                Vector2s glyph_offset = get_glyph_offset(&cursor, glyph->render_offset);
                render_glyph_bitmap_unoptimized(glyph, glyph_offset, text_region, TITLEBAR_FOREGROUND_COLOR);
            }
            advance(&cursor, 1);
        }
    }
}

internal void
draw_editor_scrollbar(Rect2D scrollbar_region, f32 offset, f32 height, ScrollbarState state)
{
    if (is_degenerated(scrollbar_region))
        return;

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
draw_editor_panel(EditorPanelLayout layout, EditorPanel* panel)
{
    draw_editor_buffer(layout.buffer_region, &panel->buffer,
                       panel->cursor.state.byte_offset, panel->cursor.state.trail_byte_offset,
                       panel->first_line_offset, panel->first_column_offset);
    
    draw_editor_cursor(layout.buffer_region, panel);

    draw_editor_scrollbar(layout.scrollbar_region, panel->scrollbar_offset_percentage,
                          panel->scrollbar_height_percentage, panel->scrollbar_state);

    //
    // NOTE(Traian): Finding the max line offset and max column offset of the current buffer is really not the
    // reponsability of the rendering layer. This should obviously be handled by the update layer. However, there
    // is no transient communication medium between the two layers, and storing these values as persistent state
    // is not something I want to currently do... (9th January 2026)
    //

    u32 max_line_offset = 0;
    u32 max_column_offset = 0;
    u32 current_column_offset = 0;
    for (Utf8Iterator buffer_iterator = utf8_iterator(panel->buffer.data, panel->buffer.size);
         is_valid(buffer_iterator);
         advance(&buffer_iterator))
    {
        u32 codepoint = buffer_iterator.codepoint;
        if (codepoint == '\r') {
            // @Cleanup, @Robustness: We should really handle the LF vs CRLF line encoding more seriously
            // and consistently. There are multiple places in where we do the exact same steps as below...
            auto peek_result = peek_next(buffer_iterator);
            if (peek_result.is_valid && peek_result.codepoint == '\n')
                advance(&buffer_iterator);

            ++max_line_offset;
            current_column_offset = 0;
        } else {
            ++current_column_offset;
            max_column_offset = max(max_column_offset, current_column_offset);
        }
    }

    BufferPosition cursor = get_position_from_byte_offset(&panel->buffer, panel->cursor.state.byte_offset);
    draw_editor_titlebar(layout.titlebar_region, panel->title, cursor.line_offset, cursor.column_offset,
                         max_line_offset, max_column_offset);
}

internal void
draw_editor_splitter(Rect2D splitter_region)
{
    if (is_degenerated(splitter_region))
        return;

    // @Cleanup: It would be really neat to actually use 'rect_intersect' in order to compute this regions!
    Rect2D titlebar_intersection_region = {};
    titlebar_intersection_region.min = splitter_region.min;
    titlebar_intersection_region.max.x = splitter_region.max.x;
    titlebar_intersection_region.max.y = splitter_region.min.y + TITLEBAR_SIZE;
    render_quad_opaque_unoptimized(titlebar_intersection_region, TITLEBAR_BACKGROUND_COLOR);

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

    bool single_panel_has_scrollbar = (state->first_panel.scrollbar_state  != ScrollbarState_Hidden);
    bool left_panel_has_scrollbar   = (state->first_panel.scrollbar_state  != ScrollbarState_Hidden);
    bool right_panel_has_scrollbar  = (state->second_panel.scrollbar_state != ScrollbarState_Hidden);
    EditorLayout editor_layout = compute_editor_layout(bitmap_size, single_panel_has_scrollbar,
                                                       left_panel_has_scrollbar, right_panel_has_scrollbar);

    if (state->is_splitscreen) {
        draw_editor_panel(editor_layout.left_panel, &state->first_panel);
        draw_editor_panel(editor_layout.right_panel, &state->second_panel);
        draw_editor_splitter(editor_layout.splitter_region);
    } else {
    }
}
