/*
 * Copyright (c) 2025-2026 Traian Avram. All rights reserved.
 * This file is part of my personal text editor and is distributed under the MIT license.
 */

internal EditorBuffer
allocate_editor_buffer(usize committed, usize reserved)
{
    ASSERT(committed <= reserved);
    committed = os_get_memory_page_aligned(committed);
    reserved = os_get_memory_page_aligned(reserved);

    EditorBuffer buffer = {};
    buffer.data = (u8*)os_reserve_memory(reserved);
    os_commit_memory(buffer.data, committed);
    buffer.committed_size = committed;
    buffer.reserved_size = reserved;
    return buffer;
}

internal void
ensure_capacity(EditorBuffer* buffer, usize capacity)
{
    if (buffer->committed_size >= capacity)
        return;
    ASSERT(capacity <= buffer->reserved_size);

    usize new_committed = buffer->committed_size * 2;
    if (capacity > new_committed)
        new_committed = capacity;
    new_committed = os_get_memory_page_aligned(new_committed);

    os_commit_memory(buffer->data + buffer->committed_size, new_committed - buffer->committed_size);
    buffer->committed_size = new_committed;
}

internal void
insert_into_buffer(EditorBuffer* buffer, usize offset, void* data, usize data_size)
{
    // Ensure buffer capacity.
    ASSERT(offset <= buffer->size);
    ensure_capacity(buffer, buffer->size + data_size);

    // Shift the rest of the buffer to the right.
    usize dst_offset = buffer->size + data_size - 1;
    usize src_offset = buffer->size - 1;
    for (usize copy_index = 0; copy_index < buffer->size - offset; ++copy_index) {
        buffer->data[dst_offset] = buffer->data[src_offset];
        --dst_offset;
        --src_offset;
    }

    // Copy the new data into the space created by the shift.
    copy_memory(buffer->data + offset, data, data_size);
    buffer->size += data_size;
}

internal void
remove_from_buffer(EditorBuffer* buffer, usize offset, usize size)
{
    ASSERT(offset + size <= buffer->size);

    usize dst_offset = offset;
    usize src_offset = offset + size;
    for (usize copy_index = 0; copy_index < buffer->size - (offset + size); ++copy_index) {
        buffer->data[dst_offset] = buffer->data[src_offset];
        ++dst_offset;
        ++src_offset;
    }

    zero_memory(buffer->data + buffer->size - size, size);
    buffer->size -= size;
}
internal void
initialize_editor(EditorState* state)
{
    state->first_panel.buffer = allocate_editor_buffer(MiB(1), GiB(1));
    // Load testing content for the first panel.
    OSReadFileResult read_file_result1 = os_read_entire_file("C:/Dev/editor3/src/render.cpp");
    if (read_file_result1.is_valid)
        insert_into_buffer(&state->first_panel.buffer, 0, read_file_result1.data, read_file_result1.size);
    state->first_panel.title = STRING_LIT("C:/Dev/editor3/src/render.cpp");

    state->second_panel.buffer = allocate_editor_buffer(MiB(1), GiB(1));
    // Load testing content for the second panel.
    OSReadFileResult read_file_result2 = os_read_entire_file("C:/Dev/editor3/src/update.cpp");
    if (read_file_result2.is_valid)
        insert_into_buffer(&state->second_panel.buffer, 0, read_file_result2.data, read_file_result2.size);
    state->second_panel.title = STRING_LIT("C:/Dev/editor3/src/update.cpp");
}

internal void
update_editor(EditorState* state)
{
    state->is_splitscreen = true;

    state->first_panel.scrollbar_state = ScrollbarState_Hidden;
    state->first_panel.scrollbar_offset_percentage = 0.0F;
    state->first_panel.scrollbar_height_percentage = 0.2F;
    state->second_panel.scrollbar_state = ScrollbarState_Visible;
    state->second_panel.scrollbar_offset_percentage = 0.2F;
    state->second_panel.scrollbar_height_percentage = 0.5F;
    
    state->first_panel.first_line_offset = 100;
    state->first_panel.first_column_offset = 0;
    state->first_panel.caret.line_offset = 104;
    state->first_panel.caret.column_offset = 10;
    state->first_panel.caret.has_selection = true;
    state->first_panel.caret.trail_line_offset = 104;
    state->first_panel.caret.trail_column_offset = 7;

    state->second_panel.first_line_offset = 10;
    state->second_panel.first_column_offset = 5;
    state->second_panel.caret.line_offset = 92;
    state->second_panel.caret.column_offset = 7;
}
