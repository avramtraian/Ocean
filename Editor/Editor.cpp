/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#include <Editor/Editor.h>

EditorState* editor_initialize(LinearArena* permanent_arena, Window window)
{
    EditorState* state = (EditorState*)core_linear_arena_allocate(permanent_arena, sizeof(EditorState));

    state->graphics_context = graphics_context_create(permanent_arena, GRAPHICS_CONTEXT_TYPE_CPU, window);
    state->swapchain_bitmap = graphics_context_allocate_bitmap(state->graphics_context, permanent_arena, 0, 0, GRAPHICS_BITMAP_USAGE_SWAPCHAIN);

    return state;
}

void editor_shutdown(EditorState* state)
{
    graphics_context_release_bitmap(state->graphics_context, &state->swapchain_bitmap);
    graphics_context_destroy(&state->graphics_context);

    *state = {};
}

void editor_on_update(EditorState* state)
{}
