/*
 * Copyright (c) 2025-2026 Traian Avram. All rights reserved.
 * This file is part of my personal text editor and is distributed under the MIT license.
 */

internal usize
get_line_start_offset(EditorBuffer* buffer, usize cursor_byte_offset)
{
    ASSERT(cursor_byte_offset <= buffer->size);

    for (s64 current_byte_offset = cursor_byte_offset - 1;
         current_byte_offset >= 0;
         --current_byte_offset)
    {
        if (buffer->data[current_byte_offset] == '\n') {
            usize result = current_byte_offset + sizeof('\n');
            return result;
        }
    }

    return 0;
}

// Returns the offset of the line-terminator corresponding with the line the offset currently lives on.
// If the cursor is on the last line, the returned value is equal to 'buffer.size'.
internal usize
get_line_end_offset(EditorBuffer* buffer, usize cursor_byte_offset)
{
    ASSERT(cursor_byte_offset <= buffer->size);

    for (usize current_byte_offset = cursor_byte_offset;
         current_byte_offset < buffer->size;
         ++current_byte_offset)
    {
        // @CRLF: If the current character is '\r' and the next character is '\n' we effectively reached the
        // associated line terminator and we must return from the function.
        if (buffer->data[current_byte_offset] == '\r' &&
            current_byte_offset + sizeof('\r') < buffer->size &&
            buffer->data[current_byte_offset + 1] == '\n')
        {
            return current_byte_offset;
        }

        if (buffer->data[current_byte_offset] == '\n')
            return current_byte_offset;
    }

    // We haven't found a line terminator and thus the cursor is on the last line of the buffer.
    // Return the buffer size instead, which is easy to catch/handle by the caller.
    return buffer->size;
}

// If the cursor is on the first line, the returned value is equal to the provided 'cursor_byte_offset'.
internal usize
get_next_line_offset(EditorBuffer* buffer, usize cursor_byte_offset)
{
    usize current_line_end_offset = get_line_end_offset(buffer, cursor_byte_offset);
    if (current_line_end_offset == buffer->size)
        return cursor_byte_offset; // The cursor is on the last line.

    usize line_terminator_size = sizeof('\n');
    if (buffer->data[current_line_end_offset] == '\r') {
        ASSERT(current_line_end_offset + sizeof('\r') < buffer->size);
        ASSERT(buffer->data[current_line_end_offset + 1] == '\n');
        line_terminator_size += sizeof('\r');
    }

    usize next_line_offset = current_line_end_offset + line_terminator_size;
    return next_line_offset;
}

// If the cursor is on the last line, the returned value is equal to the provided 'cursor_byte_offset'.
internal usize
get_previous_line_offset(EditorBuffer* buffer, usize cursor_byte_offset)
{
    usize current_line_start_offset = get_line_start_offset(buffer, cursor_byte_offset);
    if (current_line_start_offset == 0)
        return cursor_byte_offset; // The cursor is on the first line.

    ASSERT(buffer->data[current_line_start_offset - 1] == '\n');
    usize previous_line_offset = get_line_start_offset(buffer, current_line_start_offset - sizeof('\n'));
    return previous_line_offset;
}

internal u32
get_column_index(EditorBuffer* buffer, Font* font, u32 tab_size, usize cursor_byte_offset)
{
    ASSERT(cursor_byte_offset <= buffer->size);
    ASSERT(tab_size > 0);
    usize line_start_offset = get_line_start_offset(buffer, cursor_byte_offset);
    
    u32 current_column_index = 0;
    for (Utf8Iterator iterator = utf8_iterator(buffer->data + line_start_offset, cursor_byte_offset - line_start_offset);
         is_in_range(iterator);
         advance(&iterator))
    {
        u32 column_width = 1;

        // Calculate the tab width.
        if (codepoint_is_valid(iterator) && iterator.codepoint == '\t')
            column_width = tab_size - (current_column_index % tab_size);

        // @Incomplete: Add suport for multi-column-width glyphs.
        current_column_index += column_width;
    }

    return current_column_index;
}

internal u32
get_line_column_count(EditorBuffer* buffer, Font* font, u32 tab_size, usize line_start_offset)
{
    ASSERT(line_start_offset <= buffer->size);
    ASSERT(tab_size > 0);

    u32 column_index = 0;
    for (Utf8Iterator iterator = utf8_iterator(buffer->data + line_start_offset, buffer->size - line_start_offset);
         is_in_range(iterator);
         advance(&iterator))
    {
        // Transform the CRLF new-line sequence to LF.
        if (codepoint_is_valid(iterator) && iterator.codepoint == '\r') {
            auto peek = peek_next(iterator);
            if (peek.codepoint_is_valid && peek.codepoint == '\n')
                advance(&iterator);
        }

        if (codepoint_is_valid(iterator) && iterator.codepoint == '\n')
            return column_index;

        u32 glyph_cell_count = 6; // "<0x??>" requires 6 glyphs.
        if (codepoint_is_valid(iterator)) {
            if (iterator.codepoint == '\t')
                glyph_cell_count = tab_size - (column_index % tab_size);

            // @Incomplete: Support glyphs that require multiple cells.
            glyph_cell_count = 1;
        }

        column_index += glyph_cell_count;
    }

    // Since we haven't found a new-line character it means that this was the last line in the buffer.
    return column_index;
}

enum class SyncTrail {
    NO,
    YES,
};

enum class UpdateDesiredColumn {
    NO,
    YES,
};

internal void
set_cursor_offset(EditorBuffer* buffer, Font* font, u32 tab_size, u32 visible_column_count,
                  EditorCursor* cursor, usize new_byte_offset,
                  SyncTrail sync_trail, UpdateDesiredColumn update_desired_column)
{
    ASSERT(new_byte_offset <= buffer->size);

    cursor->head_offset = new_byte_offset;
    if (sync_trail == SyncTrail::YES)
        cursor->tail_offset = new_byte_offset;

    if (update_desired_column == UpdateDesiredColumn::YES) {
        u32 column_index = get_column_index(buffer, font, tab_size, new_byte_offset);
        cursor->desired_column_index = column_index % visible_column_count;
    }
}

internal usize
find_next_codepoint_offset(EditorBuffer* buffer, usize cursor_byte_offset)
{
    ASSERT(cursor_byte_offset <= buffer->size);
    Utf8DecodeResult decode = utf8_decode(buffer->data + cursor_byte_offset, buffer->size - cursor_byte_offset);

    if (cursor_byte_offset == buffer->size)
        return cursor_byte_offset; // There is no next codepoint.

    if (decode.is_valid) {
        // @CLRF @Cleanup: Handle CRLF line endings in a more sane and consistent way!
        if (decode.codepoint == '\r') {
            if ((cursor_byte_offset + sizeof('\r') < buffer->size) &&
                (buffer->data[cursor_byte_offset + 1] == '\n'))
            {
                // Consume both bytes.
                usize result = cursor_byte_offset + sizeof('\r') + sizeof('\n');
                return result;
            }
        }

        usize result = cursor_byte_offset + decode.byte_width;
        return result;
    }

    // Since the current byte doesn't represent the start of a valid UTF-8 encoded byte sequence,
    // advance to the next raw byte.
    usize result = cursor_byte_offset + 1;
    return result;
}

internal usize
find_previous_codepoint_offset(EditorBuffer* buffer, usize cursor_byte_offset)
{
    ASSERT(cursor_byte_offset <= buffer->size);
    Utf8DecodeResult decode = utf8_decode_reversed(buffer->data, cursor_byte_offset);

    if (cursor_byte_offset == 0)
        return cursor_byte_offset; // There is no previous codepoint.

    if (decode.is_valid) {
        // @CLRF @Cleanup: Handle CRLF line endings in a more sane and consistent way!
        if (decode.codepoint == '\n') {
            if ((cursor_byte_offset >= sizeof('\r') + sizeof('\n')) &&
                (buffer->data[cursor_byte_offset - 2] == '\r'))
            {
                // Consume both bytes.
                usize result = cursor_byte_offset - (sizeof('\r') + sizeof('\n'));
                return result;
            }
        }

        usize result = cursor_byte_offset - decode.byte_width;
        return result;
    }

    // Since the current byte doesn't represent the start of a valid UTF-8 encoded byte sequence,
    // advance to the next raw byte.
    usize result = cursor_byte_offset - 1;
    return result;
}

enum class IsSelecting {
    NO,
    YES,
};

enum class ConsumeWholeWord {
    NO,
    YES,
};

enum class WordMatchType {
    START = 0,
    LINEAR_WHITESPACE,
    NEWLINE,
    WORD,
    SPECIAL,
};

internal bool
codepoints_match(u32 current_codepoint, WordMatchType* match_type)
{
    if (*match_type == WordMatchType::START) {
        if (is_word(current_codepoint))                                  *match_type = WordMatchType::WORD;
        else if (current_codepoint == '\n' || current_codepoint == '\r') *match_type = WordMatchType::NEWLINE;
        else if (is_linear_whitespace(current_codepoint))                *match_type = WordMatchType::LINEAR_WHITESPACE;
        else                                                             *match_type = WordMatchType::SPECIAL;

        return true;
    }

    if (current_codepoint == '\n' || current_codepoint == '\r')
        return false; // Don't match multiple consecutive new-line sequences.

    if (*match_type == WordMatchType::LINEAR_WHITESPACE) {
        if (is_word(current_codepoint))                   *match_type = WordMatchType::WORD;
        else if (is_linear_whitespace(current_codepoint)) *match_type = WordMatchType::LINEAR_WHITESPACE;
        else                                              *match_type = WordMatchType::SPECIAL;

        return true;
    }

    if (is_linear_whitespace(current_codepoint))
        return (*match_type == WordMatchType::LINEAR_WHITESPACE);

    if (is_word(current_codepoint))
        return (*match_type == WordMatchType::WORD);

    return (*match_type == WordMatchType::SPECIAL);
}

internal void
move_cursor_right(EditorBuffer* buffer, Font* font, u32 tab_size, u32 visible_column_count, EditorCursor* cursor,
                  IsSelecting is_selecting, ConsumeWholeWord consume_whole_word)
{
    bool has_selection = (cursor->head_offset != cursor->tail_offset);
    usize new_cursor_offset = cursor->head_offset;

    if (has_selection && is_selecting == IsSelecting::NO && consume_whole_word == ConsumeWholeWord::NO) {
        // Set the cursor position to the end of the selection.
        new_cursor_offset = max(cursor->head_offset, cursor->tail_offset);
    } else if (cursor->head_offset < buffer->size) {
        if (consume_whole_word == ConsumeWholeWord::NO) {
            // Advance past the next codepoint.
            new_cursor_offset = find_next_codepoint_offset(buffer, cursor->head_offset);
        } else {
            u32 first_codepoint = utf8_decoded_or_raw_byte(buffer->data + cursor->head_offset,
                                                           buffer->size - cursor->head_offset);

            new_cursor_offset = cursor->head_offset;
            u32 current_codepoint = first_codepoint;

            // Advance past all codepoints that match the first codepoint.
            WordMatchType match_type = WordMatchType::START;
            while (codepoints_match(current_codepoint, &match_type)) {
                new_cursor_offset = find_next_codepoint_offset(buffer, new_cursor_offset);
                if (new_cursor_offset == buffer->size)
                    break;

                current_codepoint = utf8_decoded_or_raw_byte(buffer->data + new_cursor_offset,
                                                             buffer->size - new_cursor_offset);
            }
        }
    }

    SyncTrail sync_trail = (is_selecting == IsSelecting::YES) ? SyncTrail::NO : SyncTrail::YES;
    set_cursor_offset(buffer, font, tab_size, visible_column_count,
                      cursor, new_cursor_offset, sync_trail, UpdateDesiredColumn::YES);
}

internal void
move_cursor_left(EditorBuffer* buffer, Font* font, u32 tab_size, u32 visible_column_count, EditorCursor* cursor,
                 IsSelecting is_selecting, ConsumeWholeWord consume_whole_word)
{
    bool has_selection = (cursor->head_offset != cursor->tail_offset);
    usize new_cursor_offset = cursor->head_offset;

    if (has_selection && is_selecting == IsSelecting::NO && consume_whole_word == ConsumeWholeWord::NO) {
        // Set the cursor position to the beginning of the selection.
        new_cursor_offset = min(cursor->head_offset, cursor->tail_offset);
    } else if (cursor->head_offset > 0) {
        if (consume_whole_word == ConsumeWholeWord::NO) {
            // Devance to the previous codepoint.
            new_cursor_offset = find_previous_codepoint_offset(buffer, cursor->head_offset);
        } else {
            u32 first_codepoint = utf8_decoded_or_raw_byte_reversed(buffer->data, cursor->head_offset);

            new_cursor_offset = cursor->head_offset;
            u32 current_codepoint = first_codepoint;

            // Advance past all codepoints that match the first codepoint.
            WordMatchType match_type = WordMatchType::START;
            while (codepoints_match(current_codepoint, &match_type)) {
                new_cursor_offset = find_previous_codepoint_offset(buffer, new_cursor_offset);
                if (new_cursor_offset == 0)
                    break;

                current_codepoint = utf8_decoded_or_raw_byte_reversed(buffer->data, new_cursor_offset);
            }
        }
    }

    SyncTrail sync_trail = (is_selecting == IsSelecting::YES) ? SyncTrail::NO : SyncTrail::YES;
    set_cursor_offset(buffer, font, tab_size, visible_column_count,
                      cursor, new_cursor_offset, sync_trail, UpdateDesiredColumn::YES);
}

// Returns the byte offset (relative to the buffer start) of the column at the given index. If the
// line specified by the provided offsets doesn't have enough columns, this function will return the
// line end offset (the offset the first encountered line terminator, or buffer->size if the line is the last).
// If the column index is inside a glyph (think about tabs), the returned offset is of the first column after
// the columns associated with that glyph.
internal usize
get_column_offset(EditorBuffer* buffer, Font* font, u32 tab_size, usize line_start_offset, u32 column_index)
{
    ASSERT(line_start_offset <= buffer->size);
    ASSERT(tab_size > 0);
    
    u32 current_column_index = 0;
    u32 current_offset = line_start_offset;

    for (Utf8Iterator iterator = utf8_iterator(buffer->data + line_start_offset, buffer->size - line_start_offset);
         is_in_range(iterator);
         advance(&iterator))
    {
        if (current_column_index >= column_index)
            return current_offset;

        u32 glyph_column_width = 1;
        if (codepoint_is_valid(iterator)) {
            // @CRLF: Check for the CRLF line terminator.
            if (iterator.codepoint == '\r') {
                auto peek = peek_next(iterator);
                if (peek.codepoint_is_valid && peek.codepoint == '\n')
                    return current_offset;
            }

            // Check if we reached the current line LF terminator. 
            if (iterator.codepoint == '\n')
                return current_offset;

            if (iterator.codepoint == '\t')
                glyph_column_width = tab_size - (current_column_index % tab_size);

            // @Incomplete: Add support for multi-width glyphs.
        }

        current_column_index += glyph_column_width;
        current_offset += iterator.byte_width;
    }

    return current_offset;
}

internal u32
get_number_of_lines(void* data, usize size)
{
    if (size == 0)
        return 0;

    u8* iterator = (u8*)data;
    u8* iterator_end = iterator + size;

    u32 line_count = 1;
    while (iterator != iterator_end) {
        if (*iterator == '\n')
            ++line_count;
        ++iterator;
    }

    return line_count;
}

internal void
move_cursor_down(EditorBuffer* buffer, Font* font, u32 tab_size, u32 visible_column_count,
                 EditorCursor* cursor, IsSelecting is_selecting)
{
    usize current_line_offset = get_line_start_offset(buffer, cursor->head_offset);
    u32 line_column_count = get_line_column_count(buffer, font, tab_size, current_line_offset);
    u32 cursor_column_index = get_column_index(buffer, font, tab_size, cursor->head_offset); // @Speed: This also calculates 'current_line_offset' which is not very good for performance.

    u32 wrapped_line_count = (line_column_count + visible_column_count - 1) / visible_column_count; // How many rendering lines does the current line require in order to be rendered.
    u32 wrapped_line_index = cursor_column_index / visible_column_count; // On which rendering line is the cursor currently on.

    usize new_cursor_offset;
    if (wrapped_line_index + 1 < wrapped_line_count) {
        // @Incomplete: This doesn't take into consideration multi-width glyphs (such as tabs) that don't fit
        // entirely at the end of the current rendering line and thus waste a few cells, which would cause the
        // cursor to go too far to the right.
        new_cursor_offset = get_column_offset(buffer, font, tab_size, cursor->head_offset, visible_column_count);
    } else {
        // We are on the last rendering line of the wrapped line, meaning we should jump to the next buffer line.

        new_cursor_offset = get_next_line_offset(buffer, cursor->head_offset);
        if (new_cursor_offset != cursor->head_offset) {
            // This function handles the cases when there are not enough columns on the next line, as
            // well as when the desired column index is inside a glyph.
            new_cursor_offset = get_column_offset(buffer, font, tab_size, new_cursor_offset,
                                                  cursor->desired_column_index);
        }
    }

    SyncTrail sync_trail = (is_selecting == IsSelecting::YES) ? SyncTrail::NO : SyncTrail::YES;
    set_cursor_offset(buffer, font, tab_size, visible_column_count,
                      cursor, new_cursor_offset, sync_trail, UpdateDesiredColumn::NO);
}

internal void
move_cursor_up(EditorBuffer* buffer, Font* font, u32 tab_size, u32 visible_column_count,
               EditorCursor* cursor, IsSelecting is_selecting)
{
    u32 cursor_column_index = get_column_index(buffer, font, tab_size, cursor->head_offset);
    u32 wrapped_line_index = cursor_column_index / visible_column_count; // On which rendering line is the cursor currently on.

    usize new_cursor_offset;
    if (wrapped_line_index > 0) {
        usize current_line_offset = get_line_start_offset(buffer, cursor->head_offset);
        // @Incomplete: This doesn't take into consideration multi-width glyphs (such as tabs) that don't fit
        // entirely at the end of the current rendering line and thus waste a few cells, which would cause the
        // cursor to go too far to the right.
        u32 new_column_index = (wrapped_line_index - 1) * visible_column_count + cursor->desired_column_index;
        new_cursor_offset = get_column_offset(buffer, font, tab_size, current_line_offset, new_column_index);
    } else {
        usize previous_line_offset = get_previous_line_offset(buffer, cursor->head_offset);
        if (previous_line_offset != cursor->head_offset) {
            u32 prev_line_column_count = get_line_column_count(buffer, font, tab_size, previous_line_offset);
            u32 prev_wrapped_line_count = (prev_line_column_count + visible_column_count - 1) / visible_column_count;

            if (prev_wrapped_line_count == 1) {
                // This function handles the cases when there are not enough columns on the previous line, as
                // well as when the desired column index is inside a glyph.
                new_cursor_offset = get_column_offset(buffer, font, tab_size, previous_line_offset,
                                                      cursor->desired_column_index);
            } else {
                u32 new_column_index = (prev_wrapped_line_count - 1) * visible_column_count + cursor->desired_column_index;
                new_cursor_offset = get_column_offset(buffer, font, tab_size, previous_line_offset, new_column_index);
            }
        } else {
            new_cursor_offset = cursor->head_offset;
        }
    }

    SyncTrail sync_trail = (is_selecting == IsSelecting::YES) ? SyncTrail::NO : SyncTrail::YES;
    set_cursor_offset(buffer, font, tab_size, visible_column_count,
                      cursor, new_cursor_offset, sync_trail, UpdateDesiredColumn::NO);
}

internal void
remove_cursor_unordered(EditorCursor* cursors, u32 cursor_count, u32 cursor_index)
{
    if (cursor_index + 1 >= cursor_count) {
        // We want to remove the last cursor. No operation required.
    } else {
        // Move the last cursor into the removed cursor slot.
        cursors[cursor_index] = cursors[cursor_count - 1];
    }
}

// Returns the new number of cursors. The function doesn't preserve the order of the cursors,
// since it moves the alive cursors to the front of the initial cursor array.
internal u32
merge_overlapping_cursors(EditorCursor* cursors, u32 cursor_count)
{
    if (cursor_count <= 1)
        return cursor_count;

    // @Cleanup: Make this validation step nicer or remove it if it turns out this assumption is not always true,
    // or it isn't required by the following merge implementation.
    bool trails_are_before_heads = cursors[0].head_offset >= cursors[0].tail_offset;
    for (u32 index = 0; index < cursor_count; ++index) {
        EditorCursor* cursor = cursors + index;
        if (cursors->head_offset != cursors->tail_offset) { // Since there is no well-defined order.
            bool cursor_trail_order = cursors->head_offset >= cursors->tail_offset;
            ASSERT(cursor_trail_order == trails_are_before_heads);
        }
    }

    u32 current_cursor_count = cursor_count;

    //
    // NOTE(Traian): @Performance: This O(n^2) loop is not ideal, but how many cursors does a user have
    // at a single point in time? I can't recall ever using more than 50 cursors... (14th January 2026)
    //

    for (int i = 0; i < current_cursor_count - 1; ++i) {
        for (int j = i + 1; j < current_cursor_count; ++j) {
            usize a0 = min(cursors[i].head_offset, cursors[i].tail_offset);
            usize a1 = max(cursors[i].head_offset, cursors[i].tail_offset);

            usize b0 = min(cursors[j].head_offset, cursors[j].tail_offset);
            usize b1 = max(cursors[j].head_offset, cursors[j].tail_offset);

            if (a0 <= b0 && b0 < a1) {
                // Cursor order: A .. B
                if (trails_are_before_heads) {
                    // We are selecting "forward". Keep B and merge A into it.
                    cursors[j].tail_offset = cursors[i].tail_offset;
                    remove_cursor_unordered(cursors, current_cursor_count, i);
                    --current_cursor_count;
                    --i;
                    break; // Break out of the j loop.
                } else {
                    // We are selecting "backwards". Keep A and merge B into it.
                    cursors[i].tail_offset = cursors[j].tail_offset;
                    remove_cursor_unordered(cursors, cursor_count, j);
                    --current_cursor_count;
                    --j;
                }
            } else if (b0 <= a0 && a0 < b1) {
                // Cursor order: B .. A
                if (trails_are_before_heads) {
                    // We are selecting "forward". Keep A and merge B into it.
                    cursors[i].tail_offset = cursors[j].tail_offset;
                    remove_cursor_unordered(cursors, cursor_count, j);
                    --current_cursor_count;
                    --j;
                } else {
                    // We are selecting "backwards". Keep B and merge A into it.
                    cursors[j].tail_offset = cursors[i].tail_offset;
                    remove_cursor_unordered(cursors, current_cursor_count, i);
                    --current_cursor_count;
                    --i;
                    break; // Break out of the j loop.
                }
            }
        }
    }

    return current_cursor_count;
}

internal u32
spawn_cursor_at_offset(EditorBuffer* buffer, usize cursor_byte_offset, u32 desired_column_index)
{
    ASSERT(buffer->cursor_count < buffer->cursor_allocated_count); // @Incomplete!
    u32 cursor_index = buffer->cursor_count++;
    EditorCursor* cursor = buffer->cursors + cursor_index;

    cursor->head_offset = cursor_byte_offset;
    cursor->tail_offset = cursor_byte_offset;
    cursor->desired_column_index = desired_column_index;

    return cursor_index;
}

internal void
destroy_extra_cursors(EditorBuffer* buffer)
{
    // Destroy all but the first cursor. Note that there is nothing special to the first cursor, but
    // this makes the implementation trivial and the user usually doesn't care about which cursor
    // remains alive (hopefully?).
    buffer->cursor_count = 1;
}

internal void
update_navigation_system(EditorState* state, FrameInput* frame_input)
{
    EditorPanel* panel = state->active_panel;
    EditorBuffer* buffer = &state->active_panel->content_buffer;
    Font* text_font = font_from_id(FontID::TEXT_REGULAR);

    u32 view_column_count = UINT32_MAX;
    if (panel->wrap_content_lines) {
        EditorPanelLayout layout = get_panel_layout(LayoutType::SINGLE, true, false); // @Incomplete!
        view_column_count = rect_size_x(layout.content_region) / text_font->glyph_cell_size.x;
    }
    ASSERT(view_column_count > 0);

    for (u32 cursor_index = 0; cursor_index < buffer->cursor_count; ++cursor_index) {
        EditorCursor* cursor = buffer->cursors + cursor_index;
        cursor->desired_column_index = cursor->desired_column_index % view_column_count;
    }

    u32 buffer_line_count = get_number_of_lines(buffer->data, buffer->size);
    
    // Move the existing cursors:
    {
        EditorCursor* cursors = buffer->cursors;
        u32 cursor_count = buffer->cursor_count;

        IsSelecting cursor_is_selecting = (frame_input->keys[KeyCode_Shift].is_down)
                                            ? IsSelecting::YES
                                            : IsSelecting::NO;

        ConsumeWholeWord cursor_consume_whole_word = (frame_input->keys[KeyCode_Control].is_down)
                                                        ? ConsumeWholeWord::YES
                                                        : ConsumeWholeWord::NO;

        // Move cursor right:
        for (u32 i = 0; i < frame_input->keys[KeyCode_Right].event_count; ++i) {
            for (u32 cursor_index = 0; cursor_index < cursor_count; ++cursor_index) {
                EditorCursor* cursor = cursors + cursor_index;
                move_cursor_right(buffer, text_font, TAB_SIZE, view_column_count,
                                  cursor, cursor_is_selecting, cursor_consume_whole_word);
            }
        }

        // Move cursor left:
        for (u32 i = 0; i < frame_input->keys[KeyCode_Left].event_count; ++i) {
            for (u32 cursor_index = 0; cursor_index < cursor_count; ++cursor_index) {
                EditorCursor* cursor = cursors + cursor_index;
                move_cursor_left(buffer, text_font, TAB_SIZE, view_column_count,
                                 cursor, cursor_is_selecting, cursor_consume_whole_word);
            }
        }

        if (!frame_input->keys[KeyCode_Control].is_down) {
            // Move cursor down:
            for (u32 i = 0; i < frame_input->keys[KeyCode_Down].event_count; ++i) {
                for (u32 cursor_index = 0; cursor_index < cursor_count; ++cursor_index) {
                    EditorCursor* cursor = cursors + cursor_index;
                    // @Incomplete: Specify the visible column count and properly set the wrap lines flag.
                    move_cursor_down(buffer, text_font, TAB_SIZE, view_column_count, cursor, cursor_is_selecting);
                }
            }

            // Move cursor up:
            for (u32 i = 0; i < frame_input->keys[KeyCode_Up].event_count; ++i) {
                for (u32 cursor_index = 0; cursor_index < cursor_count; ++cursor_index) {
                    EditorCursor* cursor = cursors + cursor_index;
                    // @Incomplete: Specify the visible column count and properly set the wrap lines flag.
                    move_cursor_up(buffer, text_font, TAB_SIZE, view_column_count, cursor, cursor_is_selecting);
                }
            }
        }
    }

    // Spawn new cursors:
    {
        if (frame_input->keys[KeyCode_Control].is_down &&
            frame_input->keys[KeyCode_Alt].is_down)
        {
            for (u32 i = 0; i < frame_input->keys[KeyCode_Down].event_count; ++i) {
                u32 src_cursor_index = buffer->cursor_count - 1;
                EditorCursor* src_cursor = buffer->cursors + src_cursor_index;

                // Duplicate the source cursor.
                u32 new_cursor_index = spawn_cursor_at_offset(buffer, src_cursor->head_offset, 
                                                              src_cursor->desired_column_index);
                EditorCursor* new_cursor = buffer->cursors + new_cursor_index;

                // @Incomplete: Specify the visible column count and properly set the wrap lines flag.
                move_cursor_down(buffer, text_font, TAB_SIZE, view_column_count, new_cursor, IsSelecting::NO);
            }
        }
    }

    // Destroy cursors:
    {
        EditorCursor* cursors = buffer->cursors;
        u32 cursor_count = buffer->cursor_count;

        u32 new_cursor_count = merge_overlapping_cursors(cursors, cursor_count);
        buffer->cursor_count = new_cursor_count;

        if (frame_input->keys[KeyCode_Escape].was_pressed_this_frame)
            destroy_extra_cursors(buffer);
    }

    // Move buffer view:
    if (frame_input->keys[KeyCode_Control].is_down &&
        !frame_input->keys[KeyCode_Alt].is_down)
    {
        for (u32 i = 0; i < frame_input->keys[KeyCode_Down].event_count; ++i) {
            state->active_panel->content_view_line_index = clamp((s32)state->active_panel->content_view_line_index + 1,
                                                                 (s32)0,
                                                                 (s32)buffer_line_count - 2);
        }

        for (u32 i = 0; i < frame_input->keys[KeyCode_Up].event_count; ++i) {
            state->active_panel->content_view_line_index = clamp((s32)state->active_panel->content_view_line_index - 1,
                                                                 (s32)0,
                                                                 (s32)buffer_line_count - 2);
        }
    }
}
