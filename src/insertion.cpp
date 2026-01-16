/*
 * Copyright (c) 2025-2026 Traian Avram. All rights reserved.
 * This file is part of my personal text editor and is distributed under the MIT license.
 */

internal void
insert_in_buffer(EditorBuffer* buffer, Font* font, u32 tab_size, u32 visible_column_count,
                 usize insertion_offset, void* inserted_data, usize inserted_data_size)
{
    ASSERT(buffer->size + inserted_data_size <= buffer->committed);
    ASSERT(insertion_offset <= buffer->size);

    // Insert the data into the buffer.
    copy_memory_reversed(buffer->data + insertion_offset + inserted_data_size,
                         buffer->data + insertion_offset,
                         buffer->size - insertion_offset);
    copy_memory(buffer->data + insertion_offset, inserted_data, inserted_data_size);
    buffer->size += inserted_data_size;

    // Update cursor offsets.
    for (u32 cursor_index = 0; cursor_index < buffer->cursor_count; ++cursor_index) {
        EditorCursor* cursor = buffer->cursors + cursor_index;
        
        // Update the tail offset.
        if (cursor->tail_offset >= insertion_offset)
            cursor->tail_offset += inserted_data_size;

        if (cursor->head_offset >= insertion_offset) {
            // Update the head offset. Note that we have to call 'set_cursor_offset' in order to
            // set the desired column index correctly.
            usize new_cursor_offset = cursor->head_offset + inserted_data_size;
            set_cursor_offset(buffer, font, tab_size, visible_column_count, cursor, new_cursor_offset,
                              SyncTrail::NO, UpdateDesiredColumn::YES);
        }
    }
}

internal void
remove_from_buffer(EditorBuffer* buffer, Font* font, u32 tab_size, u32 visible_column_count,
                   usize remove_offset, usize removed_data_size)
{
    ASSERT(remove_offset <= buffer->size);
    ASSERT(removed_data_size <= buffer->size);
    ASSERT(remove_offset + removed_data_size <= buffer->size);

    // Remove the data from the buffer.
    copy_memory(buffer->data + remove_offset,
                buffer->data + remove_offset + removed_data_size,
                buffer->size - (remove_offset + removed_data_size));

    // Update cursor offsets.
    for (u32 cursor_index = 0; cursor_index < buffer->cursor_count; ++cursor_index) {
        EditorCursor* cursor = buffer->cursors + cursor_index;
        
        // Update the tail offset.
        if (cursor->tail_offset > remove_offset)
            cursor->tail_offset -= removed_data_size;

        if (cursor->head_offset > remove_offset) {
            // Update the head offset. Note that we have to call 'set_cursor_offset' in order to
            // set the desired column index correctly.
            usize new_cursor_offset = cursor->head_offset - removed_data_size;
            set_cursor_offset(buffer, font, tab_size, visible_column_count, cursor, new_cursor_offset,
                              SyncTrail::NO, UpdateDesiredColumn::YES);
        }
    }
}

internal void
delete_cursor_selection_range(EditorBuffer* buffer, Font* font, u32 tab_size, u32 visible_column_count,
                              EditorCursor* cursor)
{
    if (cursor->head_offset != cursor->tail_offset) {
        CursorSelectionRange selection = get_selection_range(cursor);
        remove_from_buffer(buffer, font, tab_size, visible_column_count,
                           selection.start_offset, selection.end_offset - selection.start_offset);

        set_cursor_offset(buffer, font, tab_size, visible_column_count,
                          cursor, selection.start_offset, SyncTrail::YES, UpdateDesiredColumn::YES);
    }
}

internal void
update_insertion_system(EditorState* state, FrameInput* frame_input)
{
    EditorPanel* panel = state->active_panel;
    EditorBuffer* buffer = &panel->content_buffer;
    Font* font = font_from_id(FontID_Text);

    u32 view_column_count = UINT32_MAX;
    if (panel->wrap_content_lines) {
        EditorPanelLayout layout = get_panel_layout(LayoutType::SINGLE, true, false); // @Incomplete!
        view_column_count = rect_size_x(layout.content_region) / font->glyph_cell_size.x;
    }
    ASSERT(view_column_count > 0);

    for (u32 cursor_index = 0; cursor_index < buffer->cursor_count; ++cursor_index) {
        EditorCursor* cursor = buffer->cursors + cursor_index;

        // Handle char events:
        for (u32 event_index = 0; event_index < frame_input->char_event_count; ++event_index) {
            u32 codepoint = frame_input->char_events[event_index];
            Utf8EncodeResult encode = utf8_encode(codepoint);
            if (!encode.is_valid) continue;

            delete_cursor_selection_range(buffer, font, TAB_SIZE, view_column_count, cursor);
            insert_in_buffer(buffer, font, TAB_SIZE, view_column_count,
                             cursor->head_offset, encode.data, encode.byte_width);
        }

        // Handle new-line input:
        for (u32 i = 0; i < frame_input->keys[KeyCode_Enter].event_count; ++i) {
            delete_cursor_selection_range(buffer, font, TAB_SIZE, view_column_count, cursor);
            insert_in_buffer(buffer, font, TAB_SIZE, view_column_count,
                             cursor->head_offset, "\n", sizeof('\n'));
        }

        ConsumeWholeWord delete_whole_word = frame_input->keys[KeyCode_Control].is_down
                                                ? ConsumeWholeWord::YES
                                                : ConsumeWholeWord::NO;

        // Handle tab input:
        for (u32 i = 0; i < frame_input->keys[KeyCode_Tab].event_count; ++i) {
            delete_cursor_selection_range(buffer, font, TAB_SIZE, view_column_count, cursor);
            insert_in_buffer(buffer, font, TAB_SIZE, view_column_count,
                             cursor->head_offset, "\t", sizeof('\t'));
        }

        // Handle backspace:
        for (u32 i = 0; i < frame_input->keys[KeyCode_Backspace].event_count; ++i) {
            if (cursor->head_offset == cursor->tail_offset) {
                move_cursor_left(buffer, font, TAB_SIZE, view_column_count,
                                 cursor, IsSelecting::YES, delete_whole_word);
            }

            delete_cursor_selection_range(buffer, font, TAB_SIZE, view_column_count, cursor);
        }

        // Handle delete:
        for (u32 i = 0; i < frame_input->keys[KeyCode_Delete].event_count; ++i) {
            if (cursor->head_offset == cursor->tail_offset) {
                move_cursor_right(buffer, font, TAB_SIZE, view_column_count,
                                  cursor, IsSelecting::YES, delete_whole_word);
            }

            delete_cursor_selection_range(buffer, font, TAB_SIZE, view_column_count, cursor);
        }
    }
}
