/**
 * Copyright (c) 2025 Traian Avram. All rights reserved.
 * This source file is part of the Ocean text editor.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#pragma once

#include "core.h"
#include "graphics.h"

// Forward declarations.
struct EditorMemory;
struct EditorPanel;
struct EditorState;

struct EditorMemory
{
    MemoryArena *permanent_arena;
    MemoryArena *work_arena;
    MemoryArena *dynamic_resources_arena;
};

enum FontIDEnum : u8
{
    FONT_ID_DEFAULT = 0,
    FONT_ID_MAX_COUNT,
};
typedef u16 FontID;

struct EditorState
{
    EditorMemory *memory;
    Bitmap       *offscreen_bitmap;
    Font          fonts[FONT_ID_MAX_COUNT];
    EditorPanel  *panels;
    u32           panel_count;
};

function EditorState * editor_initialize(EditorMemory *memory, Bitmap *offscreen_bitmap);
function void          editor_resize    (EditorState *state, u32 new_size_x, u32 new_size_y);
function void          editor_update    (EditorState *state);
function void          editor_destroy   (EditorState *state);
