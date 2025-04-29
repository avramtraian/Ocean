/**
 * Copyright (c) 2025 Traian Avram. All rights reserved.
 * This source file is part of the Ocean text editor.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#include "core.h"
#include "editor.h"

function EditorState *
editor_initialize(EditorMemory *memory, Bitmap *offscreen_bitmap)
{
    EditorState *state = ARENA_PUSH_STRUCT(memory->permanent_arena, EditorState);
    state->memory = memory;
    state->offscreen_bitmap = offscreen_bitmap;

    editor_resize(state, offscreen_bitmap->size_x, offscreen_bitmap->size_y);
    return state;
}

function void
editor_resize(EditorState *state, u32 new_size_x, u32 new_size_y)
{ }

function void
editor_update(EditorState *state)
{ }

function void
editor_destroy(EditorState *state)
{ }
