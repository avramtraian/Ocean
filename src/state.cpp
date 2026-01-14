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
    usize byte_offset;
    usize trail_byte_offset;
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

struct EditorPanelLayout {
    Rect2D region;
    Rect2D buffer_region;
    Rect2D titlebar_region;
    Rect2D scrollbar_region;
};

struct EditorLayout {
    EditorPanelLayout single_panel;
    EditorPanelLayout left_panel;
    EditorPanelLayout right_panel;
    Rect2D splitter_region;
};

constant usize TITLEBAR_SIZE  = 22;
constant usize SCROLLBAR_SIZE = 15;
constant usize SPLITTER_SIZE  = 8;

internal EditorPanelLayout
compute_editor_panel_layout(Rect2D panel_region, bool has_scrollbar)
{
    EditorPanelLayout layout = {};
    
    layout.titlebar_region.min = panel_region.min;
    layout.titlebar_region.max.x = panel_region.max.x;
    layout.titlebar_region.max.y = panel_region.min.y + TITLEBAR_SIZE;

    layout.buffer_region.min.x = panel_region.min.x;
    layout.buffer_region.max.x = panel_region.max.x;
    layout.buffer_region.min.y = layout.titlebar_region.max.y;
    layout.buffer_region.max.y = panel_region.max.y;

    if (has_scrollbar) {
        layout.buffer_region.max.x -= SCROLLBAR_SIZE;
        layout.scrollbar_region.min.x = layout.buffer_region.max.x;
        layout.scrollbar_region.max.x = panel_region.max.x;
        layout.scrollbar_region.min.y = layout.buffer_region.min.y;
        layout.scrollbar_region.max.y = panel_region.max.y;
    }

    return layout;
}

internal EditorLayout
compute_editor_layout(Vector2u window_size, bool single_panel_has_scrollbar,
                      bool left_panel_has_scrollbar, bool right_panel_has_scrollbar)
{
    EditorLayout layout = {};

    Rect2D single_panel_region = {};
    Rect2D left_panel_region   = {};
    Rect2D right_panel_region  = {};

    // When using a single panel:
    {
        single_panel_region = rect_offset_size(0, 0, window_size.x, window_size.y);
    }

    // When in splitscreen:
    {
        s32 splitter_size = SPLITTER_SIZE;
        if (left_panel_has_scrollbar)
            splitter_size = 0; // Hide the splitter when the scrollbar is already there.
        s32 available_size_x = window_size.x - splitter_size;

        left_panel_region.min  = v2s(0, 0);
        left_panel_region.max  = v2s(available_size_x / 2, window_size.y);
        right_panel_region.min = v2s(left_panel_region.max.x + splitter_size, 0);
        right_panel_region.max = v2s(window_size.x, window_size.y);

        layout.splitter_region.min.x = left_panel_region.max.x;
        layout.splitter_region.min.y = 0;
        layout.splitter_region.max.x = right_panel_region.min.x;
        layout.splitter_region.max.y = window_size.y;
    }

    layout.single_panel = compute_editor_panel_layout(single_panel_region, single_panel_has_scrollbar);
    layout.left_panel   = compute_editor_panel_layout(left_panel_region,   left_panel_has_scrollbar);
    layout.right_panel  = compute_editor_panel_layout(right_panel_region,  right_panel_has_scrollbar);

    return layout;
}

struct EditorCursor {
    CursorState state;
    u32 desired_column_offset;
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
    EditorCursor cursor;
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
    u32 event_count;
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

    KeyCode_Tilde, KeyCode_Minus, KeyCode_Equal, KeyCode_LeftBracket, KeyCode_RightBracket,
    KeyCode_Semicolon, KeyCode_Apostrophe, KeyCode_Backslash,
    KeyCode_Comma, KeyCode_Dot, KeyCode_Slash,

    KeyCode_Space, KeyCode_Tab,
    KeyCode_Backspace, KeyCode_Delete, KeyCode_Insert,
    KeyCode_Home, KeyCode_End,
    KeyCode_Enter, KeyCode_Escape,

    KeyCode_Left, KeyCode_Right, KeyCode_Up, KeyCode_Down,
    KeyCode_PageUp, KeyCode_PageDown,

    KeyCode_F1, KeyCode_F2, KeyCode_F3, KeyCode_F4,
    KeyCode_F5, KeyCode_F6, KeyCode_F7, KeyCode_F8,
    KeyCode_F9, KeyCode_F10, KeyCode_F11, KeyCode_F12,

    KeyCode_Control, KeyCode_Alt, KeyCode_Shift,

    KeyCode_MaxEnumCount,
};

struct KeyboardInput {
    constant usize key_count = KeyCode_MaxEnumCount;
    KeyState keys[key_count];

    constant usize max_char_event_codepoints = 256;
    usize char_event_codepoint_count;
    u32 char_event_codepoints[max_char_event_codepoints];
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

struct BufferPosition {
    u32 line_offset;
    u32 column_offset;
};

internal BufferPosition
get_position_from_byte_offset(EditorBuffer* buffer, usize byte_offset)
{
    BufferPosition result = {};

    Utf8Iterator buffer_iterator = utf8_iterator(buffer->data, buffer->size);
    while (is_valid(buffer_iterator) && buffer_iterator.offset < byte_offset) {
        if (buffer_iterator.codepoint == '\n') {
            // @Cleanup!
            Utf8DecodeResult peek = peek_next(buffer_iterator);
            if (peek.is_valid && peek.codepoint == '\r')
                advance(&buffer_iterator);

            result.line_offset++;
            result.column_offset = 0;
        } else {
            result.column_offset++;
        }

        advance(&buffer_iterator);
    }

    return result;
}

internal usize
get_line_byte_offset(EditorBuffer* buffer, u32 line_offset)
{
    u32 current_line_offset = 0;
    usize current_byte_offset = 0;

    Utf8Iterator buffer_iterator = utf8_iterator(buffer->data, buffer->size);
    while (is_valid(buffer_iterator) && current_line_offset < line_offset) {
        u32 codepoint = buffer_iterator.codepoint;
        current_byte_offset += buffer_iterator.byte_width;
        advance(&buffer_iterator);

        if (codepoint == '\n') {
            // @Cleanup!
            if (is_valid(buffer_iterator) && buffer_iterator.codepoint == '\r') {
                advance(&buffer_iterator);
                current_byte_offset += buffer_iterator.byte_width;
            }

            ++current_line_offset;
        }
    }

    return current_byte_offset;
}
