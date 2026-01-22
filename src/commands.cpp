/*
 * Copyright (c) 2025-2026 Traian Avram. All rights reserved.
 * This file is part of my personal text editor and is distributed under the MIT license.
 */

internal void
post_console_message(EditorState* state, String message)
{
    free(&state->console_message);
    state->console_message = copy_string(message);
}

//
// NAVIGATION COMMANDS:
//

internal TerminateCommand
cmd_center_view(EditorState* state, EditorBufferView* buffer_view)
{
    EditorBuffer* buffer = buffer_view->buffer;
    ASSERT(buffer_view->cursor_count > 0);

    EditorCursor* cursor = &buffer_view->cursors[0];
    for (u32 cursor_index = 0; cursor_index < buffer_view->cursor_count; ++cursor_index) {
        if (buffer_view->cursors[cursor_index].head_offset < cursor->head_offset)
            cursor = buffer_view->cursors + cursor_index;
    }

    CursorPosition cursor_position = get_cursor_position(buffer, font_from_id(FontID::TEXT_REGULAR), TAB_SIZE,
                                                         cursor->head_offset);
    Vector2u view_cell_count = get_content_view_cell_count(state, buffer_view);

    // @Incomplete: This doesn't take into account previous lines that might require multiple rendering lines
    // to be rendered. This will cause the line on which the cursor is on to not be perfectly centered
    // in the best cases, or not visible at all in the worst... (18th January 2026)
    s32 content_view_line_index = (s32)cursor_position.line_index - (s32)(view_cell_count.y / 2);
    content_view_line_index = clamp_non_zero(content_view_line_index);
    buffer_view->view_line_index = content_view_line_index;

    post_console_message(state, STRING_LIT("Centered view."));
    return TerminateCommand::YES;
}

internal COMMAND_FRONTEND(cmd_center_view_frontend)
{
    ASSERT(arguments.size == 0);
    ASSERT(state->active_panel != NULL);
    return cmd_center_view(state, state->active_panel->buffer_view);
}

internal TerminateCommand
cmd_center_cursor(EditorState* state, EditorBufferView* buffer_view)
{
    EditorBuffer* buffer = buffer_view->buffer;
    Vector2u view_cell_count = get_content_view_cell_count(state, buffer_view);

    destroy_extra_cursors(buffer_view);
    ASSERT(buffer_view->cursor_count > 0);

    // @Incomplete: This doesn't take into account lines that might require multiple rendering lines
    // to be rendered. This will cause the line on which the cursor is on to not be perfectly centered
    // in the best cases, or not visible at all in the worst... (18th January 2026)
    u32 new_cursor_line = buffer_view->view_line_index + (view_cell_count.y / 2);
    usize new_cursor_offset = get_line_offset_from_index(buffer, new_cursor_line);

    EditorCursor* cursor = &buffer_view->cursors[0];
    set_cursor_offset(buffer, font_from_id(FontID::TEXT_REGULAR), TAB_SIZE, view_cell_count.x,
                      cursor, new_cursor_offset, SyncTrail::YES, UpdateDesiredColumn::YES);
    return TerminateCommand::YES;
}

internal COMMAND_FRONTEND(cmd_center_cursor_frontend)
{
    ASSERT(arguments.size == 0);
    ASSERT(state->active_panel != NULL);
    return cmd_center_cursor(state, state->active_panel->buffer_view);
}

internal TerminateCommand
cmd_page_down(EditorState* state, EditorBufferView* buffer_view)
{
    EditorBuffer* buffer = buffer_view->buffer;
    Vector2u view_cell_count = get_content_view_cell_count(state, buffer_view);

    usize current_line_offset = get_line_offset_from_index(buffer, buffer_view->view_line_index);
    u32 render_line_count = 0;
    while (render_line_count < view_cell_count.y &&
           current_line_offset < buffer->size)
    {
        u32 column_count = get_line_column_count(buffer, font_from_id(FontID::TEXT_REGULAR), TAB_SIZE, current_line_offset);
        render_line_count += (column_count + view_cell_count.x - 1) / view_cell_count.x;
        buffer_view->view_line_index++;

        current_line_offset = get_next_line_offset(buffer, current_line_offset);
    }

    cmd_center_cursor(state, buffer_view);
    return TerminateCommand::YES;
}

internal COMMAND_FRONTEND(cmd_page_down_frontend)
{
    ASSERT(arguments.size == 0);
    ASSERT(state->active_panel != NULL);
    return cmd_page_down(state, state->active_panel->buffer_view);
}

internal TerminateCommand
cmd_page_up(EditorState* state, EditorBufferView* buffer_view)
{
    EditorBuffer* buffer = buffer_view->buffer;
    Vector2u view_cell_count = get_content_view_cell_count(state, buffer_view);

    usize current_line_offset = get_line_offset_from_index(buffer, buffer_view->view_line_index);
    if (current_line_offset == 0)
        return TerminateCommand::YES;
    current_line_offset = get_previous_line_offset(buffer, current_line_offset);

    u32 render_line_count = 0;
    while (render_line_count < view_cell_count.y &&
           buffer_view->view_line_index > 0)
    {
        u32 column_count = get_line_column_count(buffer, font_from_id(FontID::TEXT_REGULAR), TAB_SIZE, current_line_offset);
        render_line_count += (column_count + view_cell_count.x - 1) / view_cell_count.x;
        
        buffer_view->view_line_index--;
        current_line_offset = get_previous_line_offset(buffer, current_line_offset);
    }

    cmd_center_cursor(state, buffer_view);
    return TerminateCommand::YES;
}

internal COMMAND_FRONTEND(cmd_page_up_frontend)
{
    ASSERT(arguments.size == 0);
    ASSERT(state->active_panel != NULL);
    return cmd_page_up(state, state->active_panel->buffer_view);
}

internal TerminateCommand
cmd_goto_line(EditorState* state, EditorBufferView* buffer_view, u32 line_index)
{
    EditorBuffer* buffer = buffer_view->buffer;
    destroy_extra_cursors(buffer_view);
    ASSERT(buffer_view->cursor_count == 1);

    usize offset = get_line_offset_from_index(buffer, line_index);
    ASSERT(offset <= buffer->size);

    set_cursor_offset(buffer, font_from_id(FontID::TEXT_REGULAR), TAB_SIZE, MAX_WRAP_COLUMN_COUNT,
                      &buffer_view->cursors[0], offset, SyncTrail::YES, UpdateDesiredColumn::YES);
    cmd_center_view(state, buffer_view);

    post_console_message(state, STRING_LIT("Go to line.")); // @Incomplete: Also print the line number.
    return TerminateCommand::YES;
}

internal COMMAND_FRONTEND(cmd_goto_line_frontend)
{
    s64 line_number = 0;
    if (!parse_integer(arguments, &line_number) || line_number <= 0 || line_number > 0xFFFFFFFF) {
        // The provided argument is not an unsigned integer.
        return TerminateCommand::NO;
    }

    u32 line_index = line_number - 1;
    ASSERT(state->active_panel != NULL);
    return cmd_goto_line(state, state->active_panel->buffer_view, line_index);
}

//
// TEXT EDIT COMMANDS:
//

internal TerminateCommand
cmd_select_all(EditorBufferView* buffer_view)
{
    destroy_extra_cursors(buffer_view);
    
    if (buffer_view->cursor_count > 0) {
        EditorBuffer* buffer = buffer_view->buffer;
        EditorCursor* cursor = &buffer_view->cursors[0];

        cursor->head_offset = buffer->size;
        cursor->tail_offset = 0;
        cursor->desired_column_index = 0;
    }

    return TerminateCommand::YES;
}

internal COMMAND_FRONTEND(cmd_select_all_frontend)
{
    ASSERT(arguments.size == 0);
    ASSERT(state->active_panel != NULL);
    return cmd_select_all(state->active_panel->buffer_view);
}

//
// FILE MANAGEMENT COMMANDS:
//

internal TerminateCommand
cmd_save_file(EditorState* state)
{
    post_console_message(state, STRING_LIT("Saved file."));
    return TerminateCommand::YES;
}

internal COMMAND_FRONTEND(cmd_save_file_frontend)
{
    ASSERT(arguments.size == 0);
    return cmd_save_file(state);
}

internal TerminateCommand
cmd_save_file_as(EditorState* state, EditorBuffer* buffer, String file_name)
{
    String null_terminated_file_name = null_terminated_frame(file_name);
    if (os_write_entire_file((char*)null_terminated_file_name.data, buffer->data, buffer->size)) {
        post_console_message(state, STRING_LIT("Saved file as."));
        return TerminateCommand::YES;
    }

    // The file path does not exist. Don't terminated the command yet.
    return TerminateCommand::NO;
}

internal COMMAND_FRONTEND(cmd_save_file_as_frontend)
{
    String file_name = arguments;
    ASSERT(state->active_panel != NULL);
    return cmd_save_file_as(state, state->active_panel->buffer_view->buffer, file_name);
}

internal TerminateCommand
cmd_open_file(EditorState* state, EditorBufferView* buffer_view, String file_name)
{
    String null_terminated_file_name = null_terminated_frame(file_name);
    OSReadFileResult read_file_result = os_read_entire_file((char*)null_terminated_file_name.data);
    if (read_file_result.is_valid) {
        clear_buffer(buffer_view);

        EditorBuffer* buffer = buffer_view->buffer;
        ensure_capacity(buffer, read_file_result.size);

        buffer->size = read_file_result.size;
        copy_memory(buffer->data, read_file_result.data, read_file_result.size);
        os_free_read_file_result(read_file_result);

        buffer_view->view_line_index = 0;
        buffer_view->view_column_index = 0;
        buffer->name = copy_string(file_name);

        post_console_message(state, STRING_LIT("Opened file."));
        return TerminateCommand::YES;
    } else {
        // The path doesn't exist or is not accessible.
        return TerminateCommand::NO;
    }
}

internal COMMAND_FRONTEND(cmd_open_file_frontend)
{
    String file_name = arguments;
    ASSERT(state->active_panel != NULL);
    return cmd_open_file(state, state->active_panel->buffer_view, file_name);
}

internal TerminateCommand
cmd_find_file(EditorState* state, EditorBufferView* buffer_view, String file_name)
{
    String absolute_file_name = {};

    // @Cleanup @Incomplete @FixMe
    absolute_file_name = concat_frame(STRING_LIT("C:/Dev/editor3/src/"), file_name);
    return cmd_open_file(state, buffer_view, absolute_file_name);
}

internal COMMAND_FRONTEND(cmd_find_file_frontend)
{
    String file_name = arguments;
    ASSERT(state->active_panel != NULL);
    return cmd_find_file(state, state->active_panel->buffer_view, file_name);
}

internal void
execute_active_command(EditorState* state)
{
    ASSERT(state->active_command != NULL);
    ASSERT(state->console_state == ConsoleState::INSERT_COMMAND_ARGUMENTS);

    EditorBuffer* console_buffer = &state->console_buffer;
    String arguments = initialize_string(console_buffer->data, console_buffer->size, StringSource_Arena);

    EditorCommand* command = state->active_command;
    TerminateCommand terminate_command = command->execute(state, arguments);

    if (terminate_command == TerminateCommand::YES) {
        clear_buffer(&state->console_buffer_view);
        state->active_command = NULL;
        state->console_state = ConsoleState::SHOW_MESSAGE;
    }
}

internal void
launch_command(EditorState* state, EditorCommand* command)
{
    state->console_state = ConsoleState::INSERT_COMMAND_ARGUMENTS;
    state->active_command = command;
    clear_buffer(&state->console_buffer_view);

    if (!command->has_arguments)
        execute_active_command(state);
}

internal void
update_command_system(EditorState* state, FrameInput* frame_input)
{
    EditorCommandTable* command_table = &state->command_table;

    if (state->console_state == ConsoleState::SHOW_MESSAGE) {
        Modifiers modifiers = MODIFIER_NONE;
        if (frame_input->keys[KeyCode_Control].is_down) modifiers |= MODIFIER_CONTROL;
        if (frame_input->keys[KeyCode_Shift].is_down)   modifiers |= MODIFIER_SHIFT;
        if (frame_input->keys[KeyCode_Alt].is_down)     modifiers |= MODIFIER_ALT;

        for (u32 command_index = 0; command_index < command_table->command_count; ++command_index) {
            EditorCommand* command = command_table->commands + command_index;
            for (u32 key_shortcut_index = 0; key_shortcut_index < command->key_shortcut_count; ++key_shortcut_index) {
                KeyShortcut key_shortcut = command->key_shortcuts[key_shortcut_index];
                if (key_shortcut.modifiers == modifiers) {
                    // @Incomplete: We should check that there are not other keys pressed at this moment.
                    if (frame_input->keys[key_shortcut.key_code].event_count > 0) {
                        launch_command(state, command);
                        return;
                    }
                }
            }
        }
    } else {
        if (frame_input->keys[KeyCode_Escape].was_pressed_this_frame) {
            // Cancel the active command or cancel typing the command name.
            state->active_command = NULL;
            state->console_state = ConsoleState::SHOW_MESSAGE;
            clear_buffer(&state->console_buffer_view);

            post_console_message(state, STRING_LIT("Canceled command."));
            return;
        }

        if (state->console_state == ConsoleState::INSERT_COMMAND_NAME &&
            frame_input->keys[KeyCode_Enter].was_pressed_this_frame)
        {
            ASSERT(state->active_command == NULL);
            EditorBuffer* console_buffer = &state->console_buffer;
            String command_name = initialize_string(console_buffer->data, console_buffer->size, StringSource_Arena);

            EditorCommand* command_to_launch = NULL;
            for (u32 command_index = 0; command_index < command_table->command_count; ++command_index) {
                EditorCommand* command = command_table->commands + command_index;
                if (compare_ascii(command_name, command->name) == StringCompare::EQUAL) {
                    command_to_launch = command;
                    break;
                }

                for (u32 alias_index = 0; alias_index < command->alias_count; ++alias_index) {
                    String alias = command->aliases[alias_index];
                    if (compare_ascii(command_name, alias) == StringCompare::EQUAL) {
                        command_to_launch = command;
                        break;
                    }
                }

                if (command_to_launch != NULL)
                    break;
            }

            if (command_to_launch) {
                launch_command(state, command_to_launch);
                return;
            }
        }

        if (state->console_state == ConsoleState::INSERT_COMMAND_ARGUMENTS &&
            frame_input->keys[KeyCode_Enter].was_pressed_this_frame)
        {
            execute_active_command(state);
        }
    }
}

internal bool
parse_key_shortcut(String string, KeyShortcut* out_shortcut)
{
    ZERO_STRUCT_POINTER(out_shortcut);

    ArrayView<String> chunks = split(string, '+');
    FOREACH_ARRAY(chunks, chunk) {
        if (compare_ascii(*chunk, STRING_LIT("ctrl")) == StringCompare::EQUAL) {
            out_shortcut->modifiers |= MODIFIER_CONTROL;
            continue;
        }

        if (compare_ascii(*chunk, STRING_LIT("shift")) == StringCompare::EQUAL) {
            out_shortcut->modifiers |= MODIFIER_SHIFT;
            continue;
        }

        if (compare_ascii(*chunk, STRING_LIT("alt")) == StringCompare::EQUAL) {
            out_shortcut->modifiers |= MODIFIER_ALT;
            continue;
        }

#define MAP_STRING_TO_KEY_CODE(x) \
    x("pgdown", KeyCode_PageDown) \
    x("pgup", KeyCode_PageUp)

#define CHECK_STRING_KEY_CODE(string, key_code_value)                               \
        if (compare_ascii(*chunk, STRING_LIT(string)) == StringCompare::EQUAL) {    \
            if (out_shortcut->key_code != KeyCode_Unknown)                          \
                return false; /* A key code was already specified. */               \
            out_shortcut->key_code = key_code_value;                                \
            continue;                                                               \
        }

        MAP_STRING_TO_KEY_CODE(CHECK_STRING_KEY_CODE);

#undef CHECK_STRING_KEY_CODE
#undef MAP_STRING_TO_KEY_CODE

        if (chunk->size != 1)
            return false;
        u32 codepoint = chunk->data[0];

        if ('a' <= codepoint && codepoint <= 'z') {
            if (out_shortcut->key_code != KeyCode_Unknown)
                return false; // A key code was already specified.
            out_shortcut->key_code = (KeyCode)(KeyCode_A + (codepoint - 'a'));
        }

        if ('0' <= codepoint && codepoint <= '9') {
            if (out_shortcut->key_code != KeyCode_Unknown)
                return false; // A key code was already specified.
            out_shortcut->key_code = (KeyCode)(KeyCode_Zero + (codepoint - '0'));
        }
    }

    return true;
}

internal void
add_key_shortcut(EditorCommand* command, char* key_shortcut)
{
    if (key_shortcut == NULL)
        return;
    ASSERT(command->key_shortcut_count < command->max_key_shortcut_count);

    String key_shortcut_string = push_string_frame(strlen(key_shortcut));
    copy_memory(key_shortcut_string.data, key_shortcut, key_shortcut_string.size);

    KeyShortcut shortcut = {};
    if (parse_key_shortcut(key_shortcut_string, &shortcut)) {
        command->key_shortcuts[command->key_shortcut_count] = shortcut;
        command->key_shortcut_count++;
    } else {
        // @Incomplete: The shortcut provided by the user is formatted incorrectly. We should however
        // report this as a warning somewhere instead of just silently discarding the key-shortcut, since
        // the user might get frustrated of why their new command doesn't run. (18th January 2026)
    }
}

internal void
add_alias(EditorCommand* command, char* alias)
{
    if (alias == NULL)
        return;
    ASSERT(command->alias_count < command->max_alias_count);

    String* alias_entry = command->aliases + command->alias_count;
    command->alias_count++;

    *alias_entry = push_string_frame(strlen(alias));
    copy_memory(alias_entry->data, alias, alias_entry->size);
}

internal EditorCommand*
add_command(EditorCommandTable* command_table, char* name, char* key_shortcut,
            bool has_arguments, PFN_command_execute* execute_function)
{
    ASSERT(command_table->command_count < command_table->max_command_count);
    EditorCommand* command = command_table->commands + command_table->command_count;
    command_table->command_count++;
    ZERO_STRUCT_POINTER(command);

    command->name = push_string(g_arenas.eternal, strlen(name));
    copy_memory(command->name.data, name, command->name.size);
    command->has_arguments = has_arguments;
    command->execute = execute_function;

    add_key_shortcut(command, key_shortcut);
    return command;
}

internal void
generate_command_table(EditorState* state)
{
    EditorCommandTable* command_table = &state->command_table;

    add_command(command_table, "center-view",   "ctrl+q",       false, cmd_center_view_frontend);
    add_command(command_table, "center-cursor", "ctrl+e",       false, cmd_center_cursor_frontend);
    add_command(command_table, "page-down",     "pgdown",       false, cmd_page_down_frontend);
    add_command(command_table, "page-up",       "pgup",         false, cmd_page_up_frontend);
    add_command(command_table, "goto-line",     "ctrl+g",       true,  cmd_goto_line_frontend);
    add_command(command_table, "select-all",    "ctrl+a",       false, cmd_select_all_frontend);
    add_command(command_table, "save-file",     "ctrl+s",       false, cmd_save_file_frontend);
    add_command(command_table, "save-file-as",  "ctrl+shift+s", true,  cmd_save_file_as_frontend);
    add_command(command_table, "open-file",     "ctrl+o",       true,  cmd_open_file_frontend);
    add_command(command_table, "find-file",     "alt+shift+o",  true,  cmd_find_file_frontend);
}
