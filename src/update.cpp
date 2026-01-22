/*
 * Copyright (c) 2025-2026 Traian Avram. All rights reserved.
 * This file is part of my personal text editor and is distributed under the MIT license.
 */

internal void
update_editor_panel(EditorState* state, FrameInput* frame_input)
{
    EditorPanel* panel = state->active_panel;
    if (panel == NULL)
        return;

    EditorBufferView* buffer_view = panel->buffer_view;
    EditorBuffer* buffer = buffer_view->buffer;

    // @Incomplete: This should be done via a key shortcut in order to allow configurability by the
    // user. However, the current implementation of the command system doesn't allow a command to
    // change the state of the console... (18th January 2026)
    if (frame_input->keys[KeyCode_Control].is_down &&
        frame_input->keys[KeyCode_Tilde].was_pressed_this_frame)
    {
        ASSERT(state->console_state == ConsoleState::SHOW_MESSAGE);
        state->console_state = ConsoleState::INSERT_COMMAND_NAME;
        clear_buffer(&state->console_buffer_view);
    }

    // @Cleanup!
    u32 view_column_count = 0;
    EditorPanelLayout layout = get_panel_layout(LayoutType::SINGLE, true, false); // @Incomplete!
    if (buffer_view->wrap_lines) {
        view_column_count = rect_size_x(layout.content_region) / font_from_id(FontID::TEXT_REGULAR)->glyph_cell_size.x;
    }
    if (view_column_count == 0)
        view_column_count = MAX_WRAP_COLUMN_COUNT; // This effecively disables any line wrapping.

    NavigationSystem navigation = {};
    navigation.buffer_view = buffer_view;
    navigation.view_column_count = view_column_count;
    navigation.font = font_from_id(FontID::TEXT_REGULAR);
    navigation.tab_size = TAB_SIZE;
    navigation.allow_mouse_cursor = true;
    navigation.buffer_region = layout.content_region;

    InsertionSystem insertion = {};
    insertion.buffer_view = buffer_view;
    insertion.view_column_count = view_column_count;
    insertion.font = font_from_id(FontID::TEXT_REGULAR);
    insertion.tab_size = TAB_SIZE;
    insertion.allow_new_lines = true;

    update_navigation_system(&navigation, frame_input);
    update_insertion_system(&insertion, frame_input);
}

internal void
update_editor_console(EditorState* state, FrameInput* frame_input)
{
    NavigationSystem navigation = {};
    navigation.buffer_view = &state->console_buffer_view;
    navigation.view_column_count = MAX_WRAP_COLUMN_COUNT;
    navigation.font = font_from_id(FontID::TEXT_REGULAR);
    navigation.tab_size = TAB_SIZE;

    InsertionSystem insertion = {};
    insertion.buffer_view = &state->console_buffer_view;
    insertion.view_column_count = MAX_WRAP_COLUMN_COUNT;
    insertion.font = font_from_id(FontID::TEXT_REGULAR);
    insertion.tab_size = TAB_SIZE;
    insertion.allow_new_lines = false;

    update_navigation_system(&navigation, frame_input);
    update_insertion_system(&insertion, frame_input);
}

internal void
update_editor(EditorState* state, FrameInput* frame_input)
{
    if (state->console_state == ConsoleState::SHOW_MESSAGE) {
        // Redirect the input to the active panel.
        update_editor_panel(state, frame_input);
    } else {
        // Redirect the input to the console system.
        update_editor_console(state, frame_input);
    }
}
