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

struct EditorState {
    GraphicsContext graphics_context;
    GraphicsBitmap swapchain_bitmap;
    DrawList draw_list;

    EditorFontTable font_table;
};

EditorState* editor_initialize(LinearArena* permanent_arena, Window window);
void editor_shutdown(EditorState* state);

void editor_on_update(EditorState* state);
