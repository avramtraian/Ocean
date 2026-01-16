/*
 * Copyright (c) 2025-2026 Traian Avram. All rights reserved.
 * This file is part of my personal text editor and is distributed under the MIT license.
 */

internal usize
get_line_offset_from_index(EditorBuffer* buffer, u32 line_index)
{
    if (line_index == 0)
        return 0;

    u32 current_line_index = 0;
    usize current_byte_offset = 0;
    
    while (current_byte_offset < buffer->size) {
        if (buffer->data[current_byte_offset] == '\n') {
            ++current_line_index;

            if (current_line_index == line_index) {
                usize line_offset = current_byte_offset + sizeof('\n');
                return line_offset;
            }
        }

        ++current_byte_offset;
    }

    return buffer->size;
}

internal GlyphRenderData*
push_glyph_to_line(LineRenderData* line_render_data)
{
    // Exploit how arenas allocate memory in order to simulate a very efficient dynamic array.
    auto* glyph = PUSH_STRUCT(g_arenas.frame, GlyphRenderData);
    if (line_render_data->glyph_count == 0)
        line_render_data->glyphs = glyph;
    line_render_data->glyph_count++;
    return glyph;
}

internal void
gather_panel_render_data(EditorPanel* panel)
{
    EditorBuffer* content_buffer = &panel->content_buffer;
    EditorBufferRenderData* render_data = &content_buffer->render_data;
    ZERO_STRUCT_POINTER(render_data);
    Font* font = font_from_id(FontID::TEXT_REGULAR);

    EditorPanelLayout layout = get_panel_layout(LayoutType::SINGLE, true, false); // @Incomplete!
    u32 view_cell_count_x = (rect_size_x(layout.content_region)) / font->glyph_cell_size.x;
    u32 view_cell_count_y = (rect_size_y(layout.content_region) + font->line_gap) / font->line_height;

    if (view_cell_count_x > 0 && view_cell_count_y > 0) {
        render_data->first_column_index = panel->content_view_column_index;
        render_data->line_count = view_cell_count_y + 2;
        render_data->lines = PUSH_ARRAY(g_arenas.frame, LineRenderData, render_data->line_count);

        u32 cell_index_x = 0;
        u32 cell_index_y = 0;
        
        u32 max_unwrapped_cell_count = view_cell_count_x;
        if (!panel->wrap_content_lines)
            max_unwrapped_cell_count = UINT32_MAX;       

        usize view_byte_offset = get_line_offset_from_index(content_buffer, panel->content_view_line_index);
        for (Utf8Iterator iterator = utf8_iterator(content_buffer->data + view_byte_offset,
                                                   content_buffer->size - view_byte_offset);
             is_in_range(iterator) && cell_index_y < render_data->line_count;
             advance(&iterator))
        {
            // Determine if the current codepoint is inside a selection range and if the cursor
            // should be rendered at the beginning of this codepoint.
            bool is_inside_selection_range = false;
            bool should_render_cursor      = false;
            usize codepoint_byte_offset = view_byte_offset + iterator.offset;
            for (u32 cursor_index = 0; cursor_index < content_buffer->cursor_count; ++cursor_index) {
                CursorSelectionRange range = get_selection_range(content_buffer->cursors + cursor_index);
                if (range.start_offset <= codepoint_byte_offset && codepoint_byte_offset < range.end_offset)
                    is_inside_selection_range = true;

                if (content_buffer->cursors[cursor_index].head_offset == codepoint_byte_offset)
                    should_render_cursor = true;
            }

            // Handle the CRLF new-line sequence.
            if (codepoint_is_valid(iterator) && iterator.codepoint == '\r') {
                auto peek = peek_next(iterator);
                if (peek.codepoint_is_valid && peek.codepoint == '\n')
                    advance(&iterator);
            }

            // Jump to the next line when encountering a LF character.
            if (codepoint_is_valid(iterator) && iterator.codepoint == '\n') {
                // Note that we push a new glyph to the line even when that would cause a line wrapping, because
                // we don't want to have a wrapped line that only contains the new-line character.
                GlyphRenderData* glyph = push_glyph_to_line(render_data->lines + cell_index_y);
                glyph->codepoint = '\n';
                glyph->cell_count = 1;
                glyph->foreground = FOREGROUND_COLOR;
                glyph->background = is_inside_selection_range ? BACKGROUND_SELECTED_COLOR : BACKGROUND_COLOR;
                if (should_render_cursor)
                    glyph->flags |= GlyphRenderFlag_HasCursor;

                cell_index_x = 0;
                ++cell_index_y;
                continue;
            }

            // Find the number of glyphs the codepoint requires to be rendered.
            u32 glyph_cell_count = 6; // "<0x??>" requires 6 glyphs.
            if (codepoint_is_valid(iterator)) {
                glyph_cell_count = 1; // @Incomplete: Support multi-width glyphs based on the font.

                if (iterator.codepoint == '\t')
                    glyph_cell_count = TAB_SIZE - (cell_index_x % TAB_SIZE);
            }

            // Wrap long lines. NOTE(Traian): When the wrap lines feature is not enabled by the user, the
            // 'max_unwrapped_cell_count' is 'UINT32_MAX' and thus the following if-condition never passes,
            // essentially disabling wrapped lines without any extra logic. (15th January 2026)
            if (cell_index_x + glyph_cell_count > max_unwrapped_cell_count) {
                render_data->lines[cell_index_y].has_end_wrap_symbol = true;
                cell_index_x = 0;
                ++cell_index_y;

                if (cell_index_y < render_data->line_count)
                    render_data->lines[cell_index_y].has_start_wrap_symbol = true;
                else
                    break;
            }

            GlyphRenderData* glyph = push_glyph_to_line(render_data->lines + cell_index_y);
            glyph->codepoint = codepoint_or_byte_value(iterator);
            glyph->cell_count = glyph_cell_count;
            glyph->foreground = FOREGROUND_COLOR;
            glyph->background = is_inside_selection_range ? BACKGROUND_SELECTED_COLOR : BACKGROUND_COLOR;

            if (!codepoint_is_valid(iterator)) glyph->flags |= GlyphRenderFlag_RawByte;
            if (should_render_cursor)          glyph->flags |= GlyphRenderFlag_HasCursor;

            cell_index_x += glyph_cell_count;
        }
    }
}

internal void
gather_render_data(EditorState* state)
{
    gather_panel_render_data(&state->first_panel);
}
