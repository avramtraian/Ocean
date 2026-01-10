/*
 * Copyright (c) 2025-2026 Traian Avram. All rights reserved.
 * This file is part of my personal text editor and is distributed under the MIT license.
 */

enum TransactionStepType : u8 {
    TransactionStepType_Insertion,
    TransactionStepType_Deletion,
};

struct TransactionStep {
    TransactionStepType type;
    usize operation_offset;
    usize operation_size;
    u8* data;
};

struct CursorState {
    u32 line_offset;
    u32 column_offset;
    bool is_selecting;
    u32 trail_line_offset;
    u32 trail_column_offset;
};

//
// Layout of a transaction memory footprint:
//   [0] Transaction structure  (sizeof(Transaction))
//   [1] Transaction steps      (sizeof(TransactionStep) * step_count)
//   [2] Initial cursor states  (sizeof(CursorState)     * cursor_count)
//   [3] Final cursor states    (sizeof(CursorState)     * cursor_count)
//   [4] Serialized step data   (variable)
//

struct Transaction {
    Transaction* next;
    Transaction* prev;
    usize allocation_size;
    u32 step_count;
    u32 cursor_count;

    // @Cleanup: These addresses can be calculated from scratch, so there is no point
    // in storing them here!
    TransactionStep* steps;
    CursorState*     initial_cursor_states;
    CursorState*     final_cursor_states;
};

struct TransactionHistory {
    // @Memory: Since we allocate one transaction history per opened file, and during an editor session
    // we can open hundreds of files, allocating a "full" ring buffer will consume way more memory than
    // any other system. One way to fix this is to grow the buffer as a dynamic array until a certain
    // threshold (e.g use a linear buffer that grows similar to a dynamic array until the transaction
    // history hits 1MiB of memory usage, then use a ring buffer of 8MiB).

    // @Memory: Another optimization we can do regarding memory consumption is to compress the transactions
    // for the common case: inserting a single character. 99% of the time the transaction is just inserting
    // a single byte (ASCII character), using a single cursor!

    OSRingBuffer buffer;
    Transaction* first_transaction;
    Transaction* last_transaction;
    Transaction* last_committed_transaction;
};

struct TransactionStepEntry {
    TransactionStepEntry* next;
    TransactionStep step;
};

struct TransactionBuilder {
    u32 step_count;
    TransactionStepEntry* first_step;
    TransactionStepEntry* last_step;
    u32 cursor_count;
    CursorState* initial_cursor_states;
    CursorState* final_cursor_states;
};

struct EditorCaret {
    u32 desired_column_offset;
    u32 line_offset;
    u32 column_offset;
    bool has_selection;
    u32 trail_line_offset;
    u32 trail_column_offset;
};

struct EditorBuffer {
    u8* data;
    usize size;
    usize committed_size;
    usize reserved_size;
};

enum ScrollbarState : u8 {
    ScrollbarState_Hidden,
    ScrollbarState_Visible, // @Cleanup: Think of a better name! This might be missleading...
    ScrollbarState_Hovered,
    ScrollbarState_InUse,
};

struct EditorPanel {
    EditorBuffer buffer;
    EditorCaret caret;
    String title;

    // @Cleanup: Transactions histories should be stored per editor buffer and not per
    // editor panel! We currently store it here because we only use it for testing purposes
    // and it's easier to access it from here (one less level of indirection).
    TransactionHistory history;

    u32 first_line_offset;
    u32 first_column_offset;

    ScrollbarState scrollbar_state;
    f32 scrollbar_offset_percentage;
    f32 scrollbar_height_percentage;
};

struct EditorState {
    bool is_splitscreen;
    EditorPanel first_panel;
    EditorPanel second_panel;
};

struct KeyState {
    bool is_down;
    bool was_pressed_this_frame;
    bool was_released_this_frame;
    bool received_key_down_event;
};

enum KeyCode : u16 {
    KeyCode_Unknown = 0,
    
    KeyCode_A, KeyCode_B, KeyCode_C, KeyCode_D,
    KeyCode_E, KeyCode_F, KeyCode_G, KeyCode_H,
    KeyCode_I, KeyCode_J, KeyCode_K, KeyCode_L,
    KeyCode_M, KeyCode_N, KeyCode_O, KeyCode_P,
    KeyCode_Q, KeyCode_R, KeyCode_S, KeyCode_T,
    KeyCode_U, KeyCode_V, KeyCode_W, KeyCode_X,
    KeyCode_Y, KeyCode_Z,

    KeyCode_Zero, KeyCode_One, KeyCode_Two, KeyCode_Three,
    KeyCode_Four, KeyCode_Five, KeyCode_Six, KeyCode_Seven,
    KeyCode_Eight, KeyCode_Nine,

    KeyCode_Left, KeyCode_Right, KeyCode_Up, KeyCode_Down,

    KeyCode_F1, KeyCode_F2, KeyCode_F3, KeyCode_F4,
    KeyCode_F5, KeyCode_F6, KeyCode_F7, KeyCode_F8,
    KeyCode_F9, KeyCode_F10, KeyCode_F11, KeyCode_F12,

    KeyCode_Control, KeyCode_Alt, KeyCode_Shift,

    KeyCode_MaxEnumCount,
};

struct KeyboardInput {
    constant usize key_count = KeyCode_MaxEnumCount;
    KeyState keys[key_count];
    u32 char_event_codepoint;
};

enum MouseButton : u8 {
    MouseButton_Unknown,
    MouseButton_Left,
    MouseButton_Right,
    MouseButton_MaxEnumCount,
};

struct MouseInput {
    constant usize button_count = MouseButton_MaxEnumCount;
    KeyState buttons[button_count];
    Vector2s position;
    f32 scroll_offset_x;
    f32 scroll_offset_y;
};

struct FrameInput {
    KeyboardInput keyboard;
    KeyboardInput mouse;
    f32 delta_time;
    f32 current_time;
};

internal usize
get_byte_offset_from_position(EditorBuffer* buffer, u32 line_offset, u32 column_offset)
{
    usize current_byte_offset = 0;
    u32 current_line_offset = 0;
    u32 current_column_offset = 0;

    while (current_byte_offset < buffer->size && current_line_offset < line_offset) {
        if (buffer->data[current_byte_offset] == '\n')
            ++current_line_offset;
        ++current_byte_offset;
    }

    while (current_byte_offset < buffer->size && current_column_offset < column_offset) {
        ++current_column_offset; // @Unicode: Properly support multi-byte encoded codepoints!
        ++current_byte_offset;
    }

    return current_byte_offset;
}
