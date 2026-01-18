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
// DRAWING THE EDITOR:
//

internal Rect2D
get_glyph_cell_region(TextCursor* cursor, u32 cell_count)
{
    Vector2s cell_size = v2s(cursor->font->glyph_cell_size.x, cursor->font->line_height);
    Vector2s cell_offset;
    cell_offset.x = cursor->current_cell_offset.x;
    cell_offset.y = cursor->current_cell_offset.y - (cursor->font->line_height - cursor->font->glyph_cell_size.y + 1) / 2;
    Rect2D cell_region = rect_offset_size(cell_offset, v2u(cell_size.x * cell_count, cell_size.y));
    return cell_region;
}

internal Rect2D
get_cursor_region(TextCursor* cursor)
{
    Vector2s cursor_size;
    cursor_size.x = cursor->font->glyph_cell_size.x * CURSOR_SIZE_PERCENTAGE_X;
    cursor_size.y = cursor->font->glyph_cell_size.y * CURSOR_SIZE_PERCENTAGE_Y;

    Rect2D cursor_region = {};
    cursor_region.min.x = cursor->current_cell_offset.x;
    cursor_region.min.y = cursor->current_cell_offset.y + (cursor->font->glyph_cell_size.y - cursor_size.y) / 2;
    cursor_region.max.x = cursor_region.min.x + cursor_size.x;
    cursor_region.max.y = cursor_region.min.y + cursor_size.y;
    return cursor_region;
}

internal void
draw_editor_buffer(EditorBufferRenderData* render_data, Rect2D region)
{
    if (is_degenerated(region))
        return;

    render_quad_opaque_unoptimized(region, BACKGROUND_COLOR);

    Font* font = font_from_id(FontID::TEXT_REGULAR);
    Vector2s cursor_start_offset = v2s(-render_data->first_column_index * font->glyph_cell_size.x, 0);
    TextCursor cursor = create_text_cursor(font, region, cursor_start_offset);

    for (u32 line_index = 0; line_index < render_data->line_count; ++line_index) {
        LineRenderData* line = render_data->lines + line_index;

        for (u32 glyph_index = 0; glyph_index < line->glyph_count; ++glyph_index) {
            GlyphRenderData* glyph_render_data = line->glyphs + glyph_index;
            u32 codepoint = glyph_render_data->codepoint;

            // Render the background.
            Rect2D cell_region = get_glyph_cell_region(&cursor, glyph_render_data->cell_count);
            cell_region = rect_intersect(cell_region, region);
            if (!is_degenerated(cell_region))
                render_quad_opaque_unoptimized(cell_region, glyph_render_data->background);

            if ('!' <= codepoint && codepoint <= '~') {
                auto* glyph = &font->ascii_grayscale_glyphs_stb[codepoint - '!'];
                Vector2s glyph_offset = get_glyph_offset(&cursor, glyph->render_offset);
                render_glyph_bitmap_unoptimized(glyph, glyph_offset, region, glyph_render_data->foreground);
            }

            if (glyph_render_data->flags & GlyphRenderFlag_HasCursor) {
                Rect2D cursor_region = get_cursor_region(&cursor);
                cursor_region = rect_intersect(cursor_region, region); // @Incomplete: The cursor region should be intersected with the panel region, not with the buffer region. When wrapping lines and when the cursor is rendered by the new-line glyph this would cause the cursor to be clipped out!
                if (!is_degenerated(cursor_region))
                    render_quad_opaque_unoptimized(cursor_region, CURSOR_COLOR);
            }

            advance(&cursor, glyph_render_data->cell_count);
        }

        if (line->has_start_wrap_symbol) {
            Rect2D symbol_region = {};
            symbol_region.min.x = region.min.x - WRAP_SYMBOL_PADDING_SIZE;
            symbol_region.max.x = region.min.x;

            u32 height = font->glyph_cell_size.y * START_WRAP_SYMBOL_SIZE_PERCENTAGE_Y;
            symbol_region.min.y = cursor.current_cell_offset.y + (font->glyph_cell_size.y - height) / 2;
            symbol_region.max.y = symbol_region.min.y + height;
            render_quad_opaque_unoptimized(symbol_region, START_WRAP_SYMBOL_COLOR);
        }
 
        if (line->has_end_wrap_symbol) {
            Rect2D symbol_region = {};
            symbol_region.min.x = region.max.x;
            symbol_region.max.x = region.max.x + WRAP_SYMBOL_PADDING_SIZE;

            u32 height = font->glyph_cell_size.y * END_WRAP_SYMBOL_SIZE_PERCENTAGE_Y;
            symbol_region.min.y = cursor.current_cell_offset.y + (font->glyph_cell_size.y - height) / 2;
            symbol_region.max.y = symbol_region.min.y + height;
            render_quad_opaque_unoptimized(symbol_region, END_WRAP_SYMBOL_COLOR);
            
        }

        next_line(&cursor);
    }
}

internal void
draw_text_line(LineRenderData* render_data, FontID font_id, Rect2D region, LinearColor cursor_color)
{
    if (is_degenerated(region))
        return;

    Font* font = font_from_id(font_id);
    TextCursor cursor = create_text_cursor(font, region, v2s(0, 0));

    for (u32 glyph_index = 0; glyph_index < render_data->glyph_count; ++glyph_index) {
        GlyphRenderData* glyph_render_data = render_data->glyphs + glyph_index;
        u32 codepoint = glyph_render_data->codepoint;

        // Render the background.
        Rect2D cell_region = get_glyph_cell_region(&cursor, glyph_render_data->cell_count);
        cell_region = rect_intersect(cell_region, region);
        if (!is_degenerated(cell_region))
            render_quad_opaque_unoptimized(cell_region, glyph_render_data->background);

        if ('!' <= codepoint && codepoint <= '~') {
            auto* glyph = &font->ascii_grayscale_glyphs_stb[codepoint - '!'];
            Vector2s glyph_offset = get_glyph_offset(&cursor, glyph->render_offset);
            render_glyph_bitmap_unoptimized(glyph, glyph_offset, region, glyph_render_data->foreground);
        }

        // Render the cursor.
        if (glyph_render_data->flags & GlyphRenderFlag_HasCursor) {
            Rect2D cursor_region = get_cursor_region(&cursor);
            cursor_region = rect_intersect(cursor_region, region);
            if (!is_degenerated(cursor_region))
                render_quad_opaque_unoptimized(cursor_region, cursor_color);
        }

        advance(&cursor, glyph_render_data->cell_count);
    }
}

internal void
draw_panel_titlebar(LineRenderData* buffer_name_render_data, LineRenderData* cursor_info_render_data,
                    Rect2D region)
{
    if (is_degenerated(region))
        return;

    render_quad_opaque_unoptimized(region, TITLEBAR_BACKGROUND_COLOR);
    
    Font* font_text       = font_from_id(FontID::TEXT_REGULAR);
    Font* font_ui_regular = font_from_id(FontID::UI_REGULAR);
    Font* font_ui_bold    = font_from_id(FontID::UI_BOLD);

    Vector2s buffer_name_size = {};
    Vector2s cursor_info_size = {};

    buffer_name_size.y = font_ui_bold->glyph_cell_size.y;
    cursor_info_size.y = font_ui_regular->glyph_cell_size.y;

    for (u32 glyph_index = 0; glyph_index < buffer_name_render_data->glyph_count; ++glyph_index)
        buffer_name_size.x += font_ui_bold->glyph_cell_size.x;

    for (u32 glyph_index = 0; glyph_index < cursor_info_render_data->glyph_count; ++glyph_index)
        cursor_info_size.x += font_ui_regular->glyph_cell_size.x;

    u32 titlebar_size = font_ui_regular->glyph_cell_size.y *
                        (1.0F + TITLEBAR_PADDING_TOP_PERCENTAGE + TITLEBAR_PADDING_BOTTOM_PERCENTAGE);
    u32 titlebar_padding_side   = font_ui_regular->glyph_cell_size.x * TITLEBAR_PADDING_SIDE_PERCENTAGE;
    u32 titlebar_padding_top    = font_ui_regular->glyph_cell_size.y * TITLEBAR_PADDING_TOP_PERCENTAGE;
    u32 titlebar_padding_bottom = font_ui_regular->glyph_cell_size.y * TITLEBAR_PADDING_BOTTOM_PERCENTAGE;

    Rect2D buffer_name_region = {};
    buffer_name_region.min.x = region.min.x + titlebar_padding_side;
    buffer_name_region.min.y = region.min.y + titlebar_padding_bottom;
    buffer_name_region.max.x = buffer_name_region.min.x + buffer_name_size.x;
    buffer_name_region.max.y = buffer_name_region.min.y + buffer_name_size.y;

    Rect2D cursor_info_region = {};
    cursor_info_region.max.x = region.max.x - titlebar_padding_side;
    cursor_info_region.min.y = region.min.y + titlebar_padding_bottom;
    cursor_info_region.min.x = cursor_info_region.max.x - cursor_info_size.x;
    cursor_info_region.max.y = cursor_info_region.min.y + cursor_info_size.y;

    buffer_name_region = rect_intersect(buffer_name_region, region);
    cursor_info_region = rect_intersect(cursor_info_region, region);

    draw_text_line(buffer_name_render_data, FontID::UI_BOLD, buffer_name_region, {});
    draw_text_line(cursor_info_render_data, FontID::UI_REGULAR, cursor_info_region, {});
}

internal void
draw_console(LineRenderData* render_data, Rect2D region)
{
    render_quad_opaque_unoptimized(region, CONSOLE_BACKGROUND_COLOR);
    Font* font = font_from_id(FontID::TEXT_REGULAR);

    Rect2D command_region = {};
    command_region.min.x = region.min.x + (font->glyph_cell_size.x * CONSOLE_PADDING_SIDE_PERCENTAGE);
    command_region.max.x = region.max.x - (font->glyph_cell_size.x * CONSOLE_PADDING_SIDE_PERCENTAGE);
    command_region.min.y = region.min.y + (font->glyph_cell_size.y * CONSOLE_PADDING_BOTTOM_PERCENTAGE);
    command_region.max.y = region.max.y - (font->glyph_cell_size.y * CONSOLE_PADDING_TOP_PERCENTAGE);

    draw_text_line(render_data, FontID::TEXT_REGULAR, command_region, CONSOLE_CURSOR_COLOR);
}

internal void
draw_editor_frame(EditorState* state)
{
    EditorPanelLayout layout = get_panel_layout(LayoutType::SINGLE, true, false); // @Incomplete!
    EditorBufferRenderData* render_data = &state->first_panel.content_buffer.render_data;

    draw_editor_buffer(render_data, layout.content_region);
    draw_panel_titlebar(&state->first_panel.buffer_name_render_data,
                        &state->first_panel.cursor_info_render_data,
                        layout.titlebar_region);

    // @Copynpaste from 'get_panel_layout'. @Cleanup!
    u32 console_height = font_from_id(FontID::TEXT_REGULAR)->glyph_cell_size.y *
                         (1.0F + CONSOLE_PADDING_TOP_PERCENTAGE + CONSOLE_PADDING_BOTTOM_PERCENTAGE);

    Rect2D console_region = {};
    console_region.min.x = 0;
    console_region.min.y = 0;
    console_region.max.x = g_window_bitmap.size_x;
    console_region.max.y = console_height;
    draw_console(&state->console_render_data, console_region);
}
