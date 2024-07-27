/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#pragma once

#include <Core/Memory/Arena.h>
#include <Graphics/DrawList.h>
#include <Graphics/Font.h>
#include <Graphics/GraphicsContext.h>
#include <Platform/Window.h>

//===============================================================================================
// EDITOR INPUT BUFFER.
//===============================================================================================

struct InputBufferCaretPosition {
    usize byte_offset;
    u32 line_number;
    u32 column_number;
};

struct InputBufferCaret {
    InputBufferCaretPosition position;
    bool is_selecting;
    // NOTE: Contains valid information only when the 'is_selecting' flag is set to true.
    InputBufferCaretPosition selection_position;
};

struct InputBuffer {
    ReadWriteByteSpan text_buffer;
    usize text_buffer_used_byte_count;
    InputBufferCaret caret;
    struct EditorCommandTable* command_table;
};

// NOTE: Returns true if the buffer was re-allocated; False otherwise.
bool input_buffer_ensure_capacity(InputBuffer* input_buffer, usize buffer_capacity);

void input_buffer_insert_characters(InputBuffer* input_buffer, usize insert_byte_offset, ReadonlyByteSpan characters_to_insert_buffer);
void input_buffer_remove_characters(InputBuffer* input_buffer, usize remove_byte_offset, usize number_of_bytes_to_remove);

//===============================================================================================
// EDITOR FONTS.
//===============================================================================================

enum EditorFontIDEnum {
    EDITOR_FONT_ID_NONE = 0,
    EDITOR_FONT_ID_TEXT,
    EDITOR_FONT_ID_UI_TEXT,

    // NOTE: Represents the number of values in the enumeration.
    EDITOR_FONT_ID_UI_COUNT,
};
typedef u8 EditorFontID;

struct EditorFontTable {
    Font* fonts;
    u32 font_count;
    // NOTE: Represents indices in the fonts array.
    u32 font_indices[EDITOR_FONT_ID_UI_COUNT];
};

//===============================================================================================
// EDITOR PANELS.
//===============================================================================================

struct EditorInputPanel {
    Rect surface;
    InputBuffer input_buffer;
    bool is_active;
};

//===============================================================================================
// EDITOR STATE.
//===============================================================================================

struct EditorState {
    GraphicsContext graphics_context;
    GraphicsBitmap swapchain_bitmap;
    DrawList draw_list;

    EditorFontTable font_table;
    GraphicsBitmap panel_bitmap;

    EditorInputPanel input_panels[4];
    // NOTE: When no panel is active this variable is set to MAX_UINT32.
    u32 focused_input_panel;
};

// NOTE: Returns a null pointer when no input buffer is in focus.
InputBuffer* editor_get_focused_input_buffer(EditorState* state);

//===============================================================================================
// EDITOR EVENT LOOP & LIFECYCLE.
//===============================================================================================

EditorState* editor_initialize(LinearArena* permanent_arena, Window window);
void editor_shutdown(EditorState* state);

void editor_on_update(EditorState* state);
