/*
 * Copyright (c) 2025-2026 Traian Avram. All rights reserved.
 * This file is part of my personal text editor and is distributed under the MIT license.
 */

internal void
ensure_capacity(EditorBuffer* buffer, usize capacity)
{
    // Make sure the buffer has enough capacity to store the inserted data.
    if (capacity > buffer->committed) {
        usize new_committed = max(2 * buffer->committed, capacity);
        new_committed = os_get_memory_page_aligned(new_committed);

        if (capacity > buffer->reserved) {
            usize new_reserved = max(2 * buffer->reserved, capacity);
            new_reserved = os_get_memory_page_aligned(new_reserved);

            u8* new_data = (u8*)os_reserve_memory(new_reserved);
            os_commit_memory(new_data, new_committed);
            copy_memory(new_data, buffer->data, buffer->size);

            os_free_memory(buffer->data);
            buffer->data = new_data;
            buffer->committed = new_committed;
            buffer->reserved = new_reserved;
        } else {
            os_commit_memory(buffer->data + buffer->committed, new_committed - buffer->committed);
            buffer->committed = new_committed;
        }
    }
}

internal void
insert_in_buffer(EditorBufferView* buffer_view, Font* font, u32 tab_size, u32 visible_column_count,
                 usize insertion_offset, void* inserted_data, usize inserted_data_size)
{
    EditorBuffer* buffer = buffer_view->buffer;

    ASSERT(insertion_offset <= buffer->size);

    ensure_capacity(buffer, buffer->size + inserted_data_size);
    
    // Insert the data into the buffer.
    copy_memory_reversed(buffer->data + insertion_offset + inserted_data_size,
                         buffer->data + insertion_offset,
                         buffer->size - insertion_offset);
    copy_memory(buffer->data + insertion_offset, inserted_data, inserted_data_size);
    buffer->size += inserted_data_size;

    // Update cursor offsets.
    for (u32 cursor_index = 0; cursor_index < buffer_view->cursor_count; ++cursor_index) {
        EditorCursor* cursor = buffer_view->cursors + cursor_index;
        
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
remove_from_buffer(EditorBufferView* buffer_view, Font* font, u32 tab_size, u32 visible_column_count,
                   usize remove_offset, usize removed_data_size)
{
    EditorBuffer* buffer = buffer_view->buffer;

    ASSERT(remove_offset <= buffer->size);
    ASSERT(removed_data_size <= buffer->size);
    ASSERT(remove_offset + removed_data_size <= buffer->size);

    // Remove the data from the buffer.
    copy_memory(buffer->data + remove_offset,
                buffer->data + remove_offset + removed_data_size,
                buffer->size - (remove_offset + removed_data_size));
    buffer->size -= removed_data_size;

    // Update cursor offsets.
    for (u32 cursor_index = 0; cursor_index < buffer_view->cursor_count; ++cursor_index) {
        EditorCursor* cursor = buffer_view->cursors + cursor_index;
        
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
clear_buffer(EditorBufferView* buffer_view)
{
    EditorBuffer* buffer = buffer_view->buffer;
    buffer->size = 0;

    if (buffer_view->cursor_count == 0)
        return;

    buffer_view->cursor_count = 1;
    buffer_view->cursors[0].head_offset = 0;
    buffer_view->cursors[0].tail_offset = 0;
    buffer_view->cursors[0].desired_column_index = 0;
}

internal void
delete_cursor_selection_range(EditorBufferView* buffer_view, Font* font, u32 tab_size, u32 visible_column_count,
                              EditorCursor* cursor)
{
    if (cursor->head_offset != cursor->tail_offset) {
        CursorSelectionRange selection = get_selection_range(cursor);
        remove_from_buffer(buffer_view, font, tab_size, visible_column_count,
                           selection.start_offset, selection.end_offset - selection.start_offset);

        set_cursor_offset(buffer_view->buffer, font, tab_size, visible_column_count,
                          cursor, selection.start_offset, SyncTrail::YES, UpdateDesiredColumn::YES);
    }
}

struct InsertionSystem {
    EditorBufferView* buffer_view;
    u32 view_column_count;
    Font* font;
    u32 tab_size;
    bool allow_new_lines;
};

internal void
update_insertion_system(InsertionSystem* system, FrameInput* frame_input)
{
    EditorBufferView* buffer_view       = system->buffer_view;
    u32               view_column_count = system->view_column_count;
    Font*             font              = system->font;
    u32               tab_size          = system->tab_size;

    EditorBuffer* buffer = buffer_view->buffer;

    for (u32 cursor_index = 0; cursor_index < buffer_view->cursor_count; ++cursor_index) {
        EditorCursor* cursor = buffer_view->cursors + cursor_index;

        // Handle char events:
        for (u32 event_index = 0; event_index < frame_input->char_event_count; ++event_index) {
            u32 codepoint = frame_input->char_events[event_index];
            Utf8EncodeResult encode = utf8_encode(codepoint);
            if (!encode.is_valid) continue;

            delete_cursor_selection_range(buffer_view, font, TAB_SIZE, view_column_count, cursor);
            insert_in_buffer(buffer_view, font, TAB_SIZE, view_column_count,
                             cursor->head_offset, encode.data, encode.byte_width);
        }

        // Handle new-line input:
        if (system->allow_new_lines) {
            for (u32 i = 0; i < frame_input->keys[KeyCode_Enter].event_count; ++i) {
                delete_cursor_selection_range(buffer_view, font, TAB_SIZE, view_column_count, cursor);
                insert_in_buffer(buffer_view, font, TAB_SIZE, view_column_count,
                                 cursor->head_offset, "\n", sizeof('\n'));
            }
        }

        ConsumeWholeWord delete_whole_word = frame_input->keys[KeyCode_Control].is_down
                                                ? ConsumeWholeWord::YES
                                                : ConsumeWholeWord::NO;

        // Handle tab input:
        for (u32 i = 0; i < frame_input->keys[KeyCode_Tab].event_count; ++i) {
            delete_cursor_selection_range(buffer_view, font, TAB_SIZE, view_column_count, cursor);
            insert_in_buffer(buffer_view, font, TAB_SIZE, view_column_count,
                             cursor->head_offset, "\t", sizeof('\t'));
        }

        // Handle backspace:
        for (u32 i = 0; i < frame_input->keys[KeyCode_Backspace].event_count; ++i) {
            if (cursor->head_offset == cursor->tail_offset) {
                move_cursor_left(buffer, font, TAB_SIZE, view_column_count,
                                 cursor, IsSelecting::YES, delete_whole_word);
            }

            delete_cursor_selection_range(buffer_view, font, TAB_SIZE, view_column_count, cursor);
        }

        // Handle delete:
        for (u32 i = 0; i < frame_input->keys[KeyCode_Delete].event_count; ++i) {
            if (cursor->head_offset == cursor->tail_offset) {
                move_cursor_right(buffer, font, TAB_SIZE, view_column_count,
                                  cursor, IsSelecting::YES, delete_whole_word);
            }

            delete_cursor_selection_range(buffer_view, font, TAB_SIZE, view_column_count, cursor);
        }
    }
}
