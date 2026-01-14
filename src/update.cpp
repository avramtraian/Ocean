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

internal void
evict_last_transactions(TransactionHistory* history)
{
    // Discard the transactions between the last committed one and the last one (if any).
    // Note that when 'last_committed_transaction' is null all history will be removed.
    while (history->last_transaction != history->last_committed_transaction) {
        Transaction* prev = history->last_transaction->prev;
        if (prev)
            prev->next = NULL;
        os_pop_newest_from_ring_buffer(&history->buffer, history->last_transaction->allocation_size);
        history->last_transaction = prev;
    }
}

internal void
evict_first_transactions_until(TransactionHistory* history, usize allocation_size)
{
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
}

struct TransactionPointers {
    Transaction*     transaction;
    TransactionStep* steps;
    CursorState*     initial_cursor_states;
    CursorState*     final_cursor_states;
    u8*              steps_data;
};

internal TransactionPointers
finalize_transaction_pointers(u8* base_address, u32 step_count, u32 cursor_count)
{
    TransactionPointers result = {};
    uintptr current_address = (uintptr)base_address;

    result.transaction = (Transaction*)current_address;
    current_address += sizeof(Transaction);

    current_address = align_to_pow2(current_address, alignof(TransactionStep));
    result.steps = (TransactionStep*)current_address;
    current_address += step_count * sizeof(TransactionStep);

    current_address = align_to_pow2(current_address, alignof(CursorState));
    result.initial_cursor_states = (CursorState*)current_address;
    result.final_cursor_states = result.initial_cursor_states + cursor_count;
    current_address += 2 * cursor_count * sizeof(CursorState);

    result.steps_data = (u8*)current_address; // No alignment is required.

    return result;
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

    // NOTE(Traian): This should happend before allocating space for the new transaction, since
    // it pops from the newest side of the ring buffer... duh! (10th January 2026)
    evict_last_transactions(history);
    evict_first_transactions_until(history, allocation_size);

    u8* transaction_address = (u8*)os_allocate_from_ring_buffer(&history->buffer, allocation_size, alignof(void*));
    ASSERT(transaction_address != NULL);

    TransactionPointers pointers = finalize_transaction_pointers(transaction_address,
                                                                 builder->step_count, builder->cursor_count);
    Transaction* transaction = pointers.transaction;

    transaction->allocation_size = allocation_size;
    transaction->step_count = builder->step_count;
    transaction->cursor_count = builder->cursor_count;

    usize step_index = 0;
    usize step_data_offset = 0;

    TransactionStepEntry* step_entry = builder->first_step;
    while (step_entry) {
        pointers.steps[step_index].type = step_entry->step.type;
        pointers.steps[step_index].operation_size = step_entry->step.operation_size;
        pointers.steps[step_index].operation_offset = step_entry->step.operation_offset;

        // Copy the operation data.
        copy_memory(pointers.steps_data + step_data_offset, step_entry->step.data, step_entry->step.operation_size);
        pointers.steps[step_index].data = pointers.steps_data + step_data_offset;
        step_data_offset += step_entry->step.operation_size;

        ++step_index;
        step_entry = step_entry->next;
    }

    copy_memory(pointers.initial_cursor_states, builder->initial_cursor_states, builder->cursor_count * sizeof(CursorState));
    copy_memory(pointers.final_cursor_states,   builder->final_cursor_states,   builder->cursor_count * sizeof(CursorState));

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
append_deletion_step(TransactionBuilder* builder, MemoryArena* arena, usize removal_offset,
                     void* removed_data, usize removed_size)
{
    TransactionStepEntry* step = PUSH_STRUCT(arena, TransactionStepEntry);
    step->step.type = TransactionStepType_Deletion;
    step->step.operation_offset = removal_offset;
    step->step.operation_size = removed_size;
    step->step.data = PUSH_ARRAY(arena, u8, removed_size);
    copy_memory(step->step.data, removed_data, removed_size);

    // @Copynpaste from 'append_insertion_step'. Should probably create a separate function that appends
    // a generic step to a transaction builder.
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
    EditorBuffer* buffer = &target_panel->buffer;
    TransactionPointers pointers = finalize_transaction_pointers((u8*)transaction, transaction->step_count,
                                                                 transaction->cursor_count);

    for (usize step_index = 0; step_index < transaction->step_count; ++step_index) {
        TransactionStep* step = pointers.steps + step_index;
        if (step->type == TransactionStepType_Insertion) {
            // Make sure the offsets are correct (or at least we don't crash).
            ASSERT(step->operation_offset <= buffer->size);
            insert_into_buffer(buffer, step->operation_offset, step->data, step->operation_size);
        } else if (step->type == TransactionStepType_Deletion) {
            // Make sure the offsets are correct (or at least we don't crash).
            ASSERT(step->operation_offset + step->operation_size <= buffer->size);

            // Make sure that the data we are set to delete from the buffer corresponds with what
            // the transactions expects to be deleted.
            u8* current_data = buffer->data + step->operation_offset;
            u8* expected_data = step->data;
            ASSERT(compare_memory(current_data, expected_data, step->operation_size) == 0);
            
            remove_from_buffer(buffer, step->operation_offset, step->operation_size);
        }
    }
}

internal void
rollback_transaction(Transaction* transaction, EditorPanel* target_panel)
{
    EditorBuffer* buffer = &target_panel->buffer;
    TransactionPointers pointers = finalize_transaction_pointers((u8*)transaction, transaction->step_count,
                                                                 transaction->cursor_count);

    for (s64 step_index = (s64)transaction->step_count - 1; step_index >= 0; --step_index) {
        TransactionStep* step = pointers.steps + step_index;
        if (step->type == TransactionStepType_Insertion) {
            // Make sure the offsets are correct (or at least we don't crash).
            ASSERT(step->operation_offset + step->operation_size <= buffer->size);

            // Make sure that the data the transaction expects to be present at that offset corresponds
            // to the data actually written in the buffer.
            u8* current_data = buffer->data + step->operation_offset;
            u8* expected_data = step->data;
            ASSERT(compare_memory(current_data, expected_data, step->operation_size) == 0);

            remove_from_buffer(buffer, step->operation_offset, step->operation_size);
        } else if (step->type == TransactionStepType_Deletion) {
            // Make sure the offsets are correct (or at least we don't crash).
            ASSERT(step->operation_offset <= buffer->size);

            insert_into_buffer(buffer, step->operation_offset, step->data, step->operation_size);
        }
    }
}

internal void
undo_history_single(TransactionHistory* history, EditorPanel* target_panel)
{
    if (history->last_committed_transaction) {
        rollback_transaction(history->last_committed_transaction, target_panel);
        history->last_committed_transaction = history->last_committed_transaction->prev;
    }
}

internal void
redo_history_single(TransactionHistory* history, EditorPanel* target_panel)
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
    state->is_splitscreen = true;
    state->active_panel = &state->first_panel;

    //
    // Initialize the first panel:
    //

    state->first_panel.buffer = allocate_editor_buffer(MiB(1), GiB(1));
    // Load testing content for the first panel.
    OSReadFileResult read_file_result1 = os_read_entire_file("C:/Dev/editor3/src/render.cpp");
    if (read_file_result1.is_valid)
        insert_into_buffer(&state->first_panel.buffer, 0, read_file_result1.data, read_file_result1.size);
    state->first_panel.title = STRING_LIT("C:/Dev/editor3/src/render.cpp");

    state->first_panel.scrollbar_state = ScrollbarState_Hidden;
    state->first_panel.scrollbar_offset_percentage = 0.0F;
    state->first_panel.scrollbar_height_percentage = 0.2F;
    state->first_panel.first_line_offset = 0;
    state->first_panel.first_column_offset = 0;
    state->first_panel.history.buffer = os_allocate_ring_buffer(MiB(4));
    state->first_panel.cursors = PUSH_ARRAY(g_arenas.eternal, EditorCursor, 16);
    state->first_panel.cursor_allocated_count = 16;
    state->first_panel.cursor_count = 2;
    state->first_panel.cursors[1].state.byte_offset = 100;
    state->first_panel.cursors[1].state.trail_byte_offset = 100;

    //
    // Initialize the second panel:
    //

    state->second_panel.buffer = allocate_editor_buffer(MiB(1), GiB(1));
    // Load testing content for the second panel.
    OSReadFileResult read_file_result2 = os_read_entire_file("C:/Dev/editor3/src/update.cpp");
    if (read_file_result2.is_valid)
        insert_into_buffer(&state->second_panel.buffer, 0, read_file_result2.data, read_file_result2.size);
    state->second_panel.title = STRING_LIT("C:/Dev/editor3/src/update.cpp");

    state->second_panel.scrollbar_state = ScrollbarState_Visible;
    state->second_panel.scrollbar_offset_percentage = 0.2F;
    state->second_panel.scrollbar_height_percentage = 0.5F;
    state->second_panel.first_line_offset = 10;
    state->second_panel.first_column_offset = 5;
    state->second_panel.history.buffer = os_allocate_ring_buffer(MiB(4));
    state->second_panel.cursors = PUSH_ARRAY(g_arenas.eternal, EditorCursor, 16);
    state->second_panel.cursor_allocated_count = 16;
    state->second_panel.cursor_count = 1;
}

internal void
update_editor(EditorState* state, FrameInput* frame_input)
{
    update_navigation_system(state, frame_input);
}
