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

internal usize
find_transaction_size(TransactionBuilder* builder)
{
    usize result = 0;
    result += sizeof(Transaction);

    // Steps.
    result = align_to_pow2(result, alignof(TransactionStep));
    result += builder->step_count * sizeof(TransactionStep);

    // Initial and final cursor states.
    result = align_to_pow2(result, alignof(CursorState));
    result += 2 * builder->cursor_count * sizeof(CursorState);

    usize steps_data_size = 0;
    for (TransactionStepEntry* entry = builder->first_step; entry != NULL; entry = entry->next)
        steps_data_size += entry->step.operation_size;

    // Steps data.
    result += steps_data_size; // No alignment is required.
    
    result = align_to_pow2(result, alignof(void*));
    return result;
}

internal void
reset_transaction_history(TransactionHistory* history)
{
    os_reset_ring_buffer(&history->buffer);
    history->first_transaction          = NULL;
    history->last_transaction           = NULL;
    history->last_committed_transaction = NULL;
}

internal Transaction*
serialize_transaction(TransactionHistory* history, TransactionBuilder* builder)
{
    usize allocation_size = find_transaction_size(builder);

    if (allocation_size > history->buffer.size) {
        // NOTE(Traian): There is no possible way we can serialize this transaction to the history. All we can do
        // is clear the existing transaction history, leaving the user without the possibility of performing any
        // unde actions. Since this erases the history, maybe we should ask them before? (9th January 2026)
        reset_transaction_history(history);
        return NULL;
    }

    // Discard the transactions between the last committed one and the last one (if any).
    // Note that when 'last_committed_transaction' is null all history will be removed.
    // NOTE(Traian): This should happend before allocating space for the new transaction, since
    // it pops from the newest side of the ring buffer... duh! (10th January 2026)
    while (history->last_transaction != history->last_committed_transaction) {
        Transaction* prev = history->last_transaction->prev;
        if (prev)
            prev->next = NULL;
        os_pop_newest_from_ring_buffer(&history->buffer, history->last_transaction->allocation_size);
        history->last_transaction = prev;
    }

    // By the time we reach this line of code, the 'last_committed_transaction' is either NULL
    // or equal to 'last_transaction'.

    if (history->buffer.used + allocation_size > history->buffer.size) { // NOTE(Traian): This is correct only when no alignment padding is introduced...
        usize available = history->buffer.size - history->buffer.used; // NOTE(Traian): Same alignment contraint here...
        usize missing = allocation_size - available;

        usize bytes_evicted_so_far = 0;
        while (bytes_evicted_so_far < missing) {
            Transaction* next = history->first_transaction->next;
            if (next)
                next->prev = NULL;
            bytes_evicted_so_far += history->first_transaction->allocation_size;
            os_pop_oldest_from_ring_buffer(&history->buffer, history->first_transaction->allocation_size);
            history->first_transaction = next;
        }

        if (history->first_transaction == NULL) {
            // We evicted all transactions, including the last one and potentially the last committed one.
            history->last_transaction = NULL;
            history->last_committed_transaction = NULL;
        }
    }

    u8* transaction_address = (u8*)os_allocate_from_ring_buffer(&history->buffer, allocation_size, alignof(void*));
    ASSERT(transaction_address != NULL);
    uintptr current_address = (uintptr)transaction_address;

    // Finalize the transaction address.
    Transaction* transaction = (Transaction*)current_address;
    current_address += sizeof(Transaction);

    // Finalize the steps address.
    current_address = align_to_pow2(current_address, alignof(TransactionStep));
    TransactionStep* steps = (TransactionStep*)current_address;
    current_address += builder->step_count * sizeof(TransactionStep);

    // Finalize the cursor initial and final states addresses.
    current_address = align_to_pow2(current_address, alignof(CursorState));
    CursorState* initial_cursor_states = (CursorState*)current_address;
    CursorState* final_cursor_states = initial_cursor_states + builder->cursor_count;
    current_address += 2 * builder->cursor_count * sizeof(CursorState);

    // Finalize the steps data buffer address.
    u8* steps_data = (u8*)current_address; // No alignment is required.

    transaction->allocation_size = allocation_size;
    transaction->step_count = builder->step_count;
    transaction->cursor_count = builder->cursor_count;

    transaction->steps = steps; // @Cleanup!
    transaction->initial_cursor_states = initial_cursor_states; // @Cleanup!
    transaction->final_cursor_states = final_cursor_states; // @Cleanup!

    usize step_index = 0;
    usize step_data_offset = 0;

    TransactionStepEntry* step_entry = builder->first_step;
    while (step_entry) {
        steps[step_index].type = step_entry->step.type;
        steps[step_index].operation_size = step_entry->step.operation_size;
        steps[step_index].operation_offset = step_entry->step.operation_offset;

        // Copy the operation data.
        copy_memory(steps_data + step_data_offset, step_entry->step.data, step_entry->step.operation_size);
        steps[step_index].data = steps_data + step_data_offset;
        step_data_offset += step_entry->step.operation_size;

        ++step_index;
        step_entry = step_entry->next;
    }

    copy_memory(initial_cursor_states, builder->initial_cursor_states, builder->cursor_count * sizeof(CursorState));
    copy_memory(final_cursor_states,   builder->final_cursor_states,   builder->cursor_count * sizeof(CursorState));

    if (history->first_transaction == NULL) {
        history->first_transaction = transaction;
        history->last_transaction = transaction;
    } else if (history->last_committed_transaction == NULL) {
        history->first_transaction = transaction;
        history->last_transaction = transaction;
    } else {
        // Append the current transaction to the end of the history.
        history->last_transaction->next = transaction;
        transaction->prev = history->last_transaction;
        history->last_transaction = transaction;
    }

    return transaction;
}

internal void
append_insertion_step(TransactionBuilder* builder, MemoryArena* arena, usize offset, void* data, usize size)
{
    TransactionStepEntry* step = PUSH_STRUCT(arena, TransactionStepEntry);
    step->step.type = TransactionStepType_Insertion;
    step->step.operation_offset = offset;
    step->step.operation_size = size;
    step->step.data = PUSH_ARRAY(arena, u8, size);
    copy_memory(step->step.data, data, size);

    if (builder->last_step)
        builder->last_step->next = step;

    if (!builder->first_step)
        builder->first_step = step;

    builder->last_step = step;
    builder->step_count++;
}

internal void
commit_transaction(Transaction* transaction, EditorPanel* target_panel)
{
    for (usize step_index = 0; step_index < transaction->step_count; ++step_index) {
        TransactionStep* step = transaction->steps + step_index;
        if (step->type == TransactionStepType_Insertion) {
            insert_into_buffer(&target_panel->buffer, step->operation_offset,
                               step->data, step->operation_size);
        }
    }
}

internal void
rollback_transaction(Transaction* transaction, EditorPanel* target_panel)
{
    for (s64 step_index = (s64)transaction->step_count - 1; step_index >= 0; --step_index) {
        TransactionStep* step = transaction->steps + step_index;
        if (step->type == TransactionStepType_Insertion) {
            remove_from_buffer(&target_panel->buffer, step->operation_offset, step->operation_size);
        }
    }
}

internal void
undo_history(TransactionHistory* history, EditorPanel* target_panel)
{
    if (history->last_committed_transaction) {
        rollback_transaction(history->last_committed_transaction, target_panel);
        history->last_committed_transaction = history->last_committed_transaction->prev;
    }
}

internal void
redo_history(TransactionHistory* history, EditorPanel* target_panel)
{
    if (history->last_committed_transaction) {
        Transaction* first_uncommitted_transaction = history->last_committed_transaction->next;
        if (first_uncommitted_transaction) {
            commit_transaction(first_uncommitted_transaction, target_panel);
            history->last_committed_transaction = first_uncommitted_transaction;
        }
    } else if (history->first_transaction) {
        commit_transaction(history->first_transaction, target_panel);
        history->last_committed_transaction = history->first_transaction;
    }
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

    state->is_splitscreen = true;

    state->first_panel.scrollbar_state = ScrollbarState_Hidden;
    state->first_panel.scrollbar_offset_percentage = 0.0F;
    state->first_panel.scrollbar_height_percentage = 0.2F;
    state->second_panel.scrollbar_state = ScrollbarState_Visible;
    state->second_panel.scrollbar_offset_percentage = 0.2F;
    state->second_panel.scrollbar_height_percentage = 0.5F;
    
    state->first_panel.first_line_offset = 0;
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

    state->first_panel.history.buffer = os_allocate_ring_buffer(MiB(4));
    state->second_panel.history.buffer = os_allocate_ring_buffer(MiB(4));
}

internal void
update_editor(EditorState* state, FrameInput* frame_input)
{
    if (frame_input->keyboard.keys[KeyCode_Right].received_key_down_event) {
        state->first_panel.caret.column_offset++;
    }

    if (frame_input->keyboard.keys[KeyCode_Left].received_key_down_event) {
        if (state->first_panel.caret.column_offset > 0)
            state->first_panel.caret.column_offset--;
        else if (state->first_panel.caret.line_offset > 0)
            state->first_panel.caret.line_offset--;
    }

    TransactionHistory* history = &state->first_panel.history;

    if (frame_input->keyboard.keys[KeyCode_A].received_key_down_event) {
        TransactionBuilder builder = {};
        append_insertion_step(&builder, g_arenas.frame, 0, "a", 1);
        Transaction* transaction = serialize_transaction(history, &builder);
        commit_transaction(transaction, &state->first_panel);
        history->last_committed_transaction = transaction;
    }
    
    /*
    ASSERT(history->first_transaction == NULL || history->first_transaction->prev == NULL);
    ASSERT(history->last_transaction == NULL || history->last_transaction->next == NULL);

    Transaction* transaction = history->first_transaction;
    while (transaction) {
        ASSERT(transaction->next == NULL || transaction->next->prev == transaction);
        if (transaction == history->last_committed_transaction)
            OutputDebugStringA(" [X] ");
        else
            OutputDebugStringA(" [ ] ");
        transaction = transaction->next;
    }
    OutputDebugStringA("\n");
    */

    if (frame_input->keyboard.keys[KeyCode_Z].received_key_down_event)
        undo_history(history, &state->first_panel);

    if (frame_input->keyboard.keys[KeyCode_Y].received_key_down_event)
        redo_history(history, &state->first_panel);
}
