/*
 * Copyright (c) 2025-2026 Traian Avram. All rights reserved.
 * This file is part of my personal text editor and is distributed under the MIT license.
 */

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
