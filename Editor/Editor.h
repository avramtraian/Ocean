/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#pragma once

#include <Core/Memory/Arena.h>
#include <Graphics/GraphicsContext.h>
#include <Platform/Window.h>

struct EditorState {
    GraphicsContext graphics_context;
    GraphicsBitmap swapchain_bitmap;
};

EditorState* editor_initialize(LinearArena* permanent_arena, Window window);
void editor_shutdown(EditorState* state);

void editor_on_update(EditorState* state);
