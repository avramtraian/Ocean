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

struct EditorPanel {
    EditorBuffer buffer;
    EditorCaret caret;

    bool is_scrollbar_visible;
    bool is_scrollbar_hovered;
    bool is_scrollbar_in_use;
    f32 scrollbar_offset_percentage;
    f32 scrollbar_height_percentage;
};

struct EditorState {
    bool is_splitscreen;
    EditorPanel first_panel;
    EditorPanel second_panel;
};
