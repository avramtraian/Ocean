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
    usize src_offset = offset + data_size - 1;
    for (usize copy_index = 0; copy_index < data_size; ++copy_index) {
        buffer->data[dst_offset] = buffer->data[src_offset];
        --dst_offset;
        --src_offset;
    }

    // Copy the new data into the space created by the shift.
    copy_memory(buffer->data + offset, data, data_size);
    buffer->size += data_size;
}

internal void
initialize_editor(EditorState* state)
{
    state->first_panel.buffer = allocate_editor_buffer(MiB(1), GiB(1));
    // Load testing content for the first panel.
    OSReadFileResult read_file_result1 = os_read_entire_file("C:/Dev/editor3/src/render.cpp");
    if (read_file_result1.is_valid)
        insert_into_buffer(&state->first_panel.buffer, 0, read_file_result1.data, read_file_result1.size);

    state->second_panel.buffer = allocate_editor_buffer(MiB(1), GiB(1));
    // Load testing content for the second panel.
    OSReadFileResult read_file_result2 = os_read_entire_file("C:/Dev/editor3/src/render.cpp");
    if (read_file_result2.is_valid)
        insert_into_buffer(&state->second_panel.buffer, 0, read_file_result2.data, read_file_result2.size);
}

internal void
update_editor(EditorState* state)
{
    state->is_splitscreen = true;
    state->first_panel.is_scrollbar_visible = true;
}
