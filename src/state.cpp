/*
 * Copyright (c) 2025-2026 Traian Avram. All rights reserved.
 * This file is part of my personal text editor and is distributed under the MIT license.
 */

//
// FRAME INPUT DEFINITIONS:
//

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

enum MouseButton : u8 {
    MouseButton_Unknown,
    MouseButton_Left,
    MouseButton_Right,
    MouseButton_Middle,
    MouseButton_MaxEnumCount,
};

struct FrameInput {
    KeyState       keys[KeyCode_MaxEnumCount];
    constant usize max_char_event_count = 256;
    u32            char_events[max_char_event_count];
    u32            char_event_count;

    KeyState mouse_buttons[MouseButton_MaxEnumCount];
    f32 mouse_wheel_vertical_scroll;
    f32 mouse_wheel_horizontal_scroll;
    Vector2s mouse_position;
};

//
// RENDERING DATA DEFINITIONS:
//

enum GlyphRenderFlagsEnum : u16 {
    GlyphRenderFlag_None      = 0,
    GlyphRenderFlag_RawByte   = BIT(0),
    GlyphRenderFlag_HasCursor = BIT(1),
};
typedef u16 GlyphRenderFlags;

struct GlyphRenderData {
    u32              codepoint;
    u16              cell_count;
    GlyphRenderFlags flags;
    LinearColor      foreground;
    LinearColor      background;
};

struct LineRenderData {
    GlyphRenderData* glyphs;
    u32              glyph_count;
    bool             has_start_wrap_symbol;
    bool             has_end_wrap_symbol;
};

struct EditorBufferRenderData {
    u32             line_count;
    LineRenderData* lines;
    u32             first_column_index;
};

//
// EDITOR STATE DEFINITIONS:
//

struct EditorCursor {
    usize head_offset;
    usize tail_offset;
    u32 desired_column_index;
};

struct CursorSelectionRange {
    usize start_offset;
    usize end_offset;
};

struct CursorPosition {
    u32 line_index;
    u32 column_index;
};

struct EditorBuffer {
    u8*                    data;
    usize                  size;
    usize                  committed;
    usize                  reserved;
    EditorCursor*          cursors;
    u32                    cursor_count;
    u32                    cursor_allocated_count;
    EditorBufferRenderData render_data;
};

struct EditorPanel {
    EditorBuffer   content_buffer;
    u32            content_view_column_index;
    u32            content_view_line_index;
    bool           wrap_content_lines;

    String buffer_name;
    LineRenderData buffer_name_render_data;
    LineRenderData cursor_info_render_data;
};

enum class ConsoleState {
    SHOW_MESSAGE,
    INSERT_COMMAND_NAME,
    INSERT_COMMAND_ARGUMENTS,
};

struct EditorState {
    EditorPanel  first_panel;
    EditorPanel  second_panel;
    EditorPanel* active_panel;

    EditorCommandTable command_table;

    ConsoleState   console_state;
    EditorCommand* active_command;
    String         console_message;
    EditorBuffer   console_buffer;
    
    LineRenderData console_render_data;
};

// Constant that will not cause any wrappings, but that it's also small enough it will not cause
// any overflow in calculations performed to determine how many rendering lines are associated with
// one buffer line.
constant u32 MAX_WRAP_COLUMN_COUNT = 1 << 31;

//
// GENERAL-PURPOSE UTILITY FUNCTIONS:
//

internal CursorPosition
get_cursor_position(EditorBuffer* buffer, Font* font, u32 tab_size, usize cursor_byte_offset)
{
    ASSERT(cursor_byte_offset <= buffer->size);
    CursorPosition result = {};

    for (Utf8Iterator iterator = utf8_iterator(buffer->data, cursor_byte_offset);
         is_in_range(iterator);
         advance(&iterator))
    {
        if (codepoint_is_valid(iterator)) {
            if (iterator.codepoint == '\n') {
                result.line_index++;
                result.column_index = 0;
            } else if (iterator.codepoint == '\t') {
                u32 tab_render_width = tab_size - (result.column_index % tab_size);
                result.column_index += tab_render_width;
            } else {
                // @Incomplete: Handle glyphs that occupy a different number of cells.
                result.column_index++;
            }
        } else {
            result.column_index += 6; // Raw byte values require 6 bytes "<0x??>" to be rendered.
        }
    }

    return result;
}

internal usize
get_line_offset_from_index(EditorBuffer* buffer, u32 line_index)
{
    if (line_index == 0)
        return 0;

    u32 current_line_index = 0;
    usize current_byte_offset = 0;
    
    while (current_byte_offset < buffer->size) {
        if (buffer->data[current_byte_offset] == '\n') {
            ++current_line_index;

            if (current_line_index == line_index) {
                usize line_offset = current_byte_offset + sizeof('\n');
                return line_offset;
            }
        }

        ++current_byte_offset;
    }

    return buffer->size;
}

internal CursorSelectionRange
get_selection_range(EditorCursor* cursor)
{
    CursorSelectionRange result;
    result.start_offset = min(cursor->head_offset, cursor->tail_offset);
    result.end_offset   = max(cursor->head_offset, cursor->tail_offset);
    return result;
}

struct EditorPanelLayout {
    Rect2D region;
    Rect2D content_region;
    Rect2D titlebar_region;
    Rect2D scrollbar_region;
};

enum LayoutType {
    SINGLE,
    SPLIT_LEFT,
    SPLIT_RIGHT,
};

internal EditorPanelLayout
get_panel_layout(LayoutType type, bool allow_line_wrapping, bool scrollbar_is_visible)
{
    Font* font_text = font_from_id(FontID::TEXT_REGULAR);
    u32 cell_count_x = (g_window_bitmap.size_x / font_text->glyph_cell_size.x);

    u32 content_size_x = cell_count_x * font_text->glyph_cell_size.x;
    while (content_size_x + 2 * WRAP_SYMBOL_PADDING_SIZE > g_window_bitmap.size_x && cell_count_x > 0) {
        --cell_count_x;
        content_size_x -= font_text->glyph_cell_size.x;
    }

    Font* font_ui = font_from_id(FontID::UI_REGULAR);
    u32 titlebar_height = font_ui->glyph_cell_size.y *
                          (1.0F + TITLEBAR_PADDING_TOP_PERCENTAGE + TITLEBAR_PADDING_BOTTOM_PERCENTAGE);

    u32 console_height = font_text->glyph_cell_size.y *
                         (1.0F + CONSOLE_PADDING_TOP_PERCENTAGE + CONSOLE_PADDING_BOTTOM_PERCENTAGE);

    EditorPanelLayout layout = {};

    layout.content_region.min.x = (g_window_bitmap.size_x - content_size_x) / 2;
    layout.content_region.min.y = console_height + titlebar_height;
    layout.content_region.max.y = g_window_bitmap.size_y;
    layout.content_region.max.x = layout.content_region.min.x + content_size_x;

    layout.titlebar_region.min.x = 0;
    layout.titlebar_region.min.y = console_height;
    layout.titlebar_region.max.x = g_window_bitmap.size_x;
    layout.titlebar_region.max.y = console_height + titlebar_height;

    return layout;
}

internal Vector2u
get_content_view_cell_count(EditorState* state, EditorPanel* panel)
{
    EditorPanelLayout layout = get_panel_layout(LayoutType::SINGLE, true, false); // @Incomplete!

    Vector2u result = {};
    if (panel->wrap_content_lines)
        result.x = rect_size_x(layout.content_region) / font_from_id(FontID::TEXT_REGULAR)->glyph_cell_size.x;
    if (result.x == 0)
        result.x = MAX_WRAP_COLUMN_COUNT; // This effecively disables any line wrapping.
    result.y = rect_size_y(layout.content_region) / font_from_id(FontID::TEXT_REGULAR)->line_height;

    return result;
}
