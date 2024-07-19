/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#include <Core/Memory/Arena.h>
#include <Core/Memory/MemoryOperations.h>
#include <Draw/Bitmap.h>
#include <Editor/State.h>
#include <Platform/OffscreenBitmap.h>
#include <Platform/Window.h>

static i32 guarded_main()
{
    LinearArena arena = {};
    core_linear_arena_initialize(&arena, 16 * MiB);

    Window window = platform_window_create(&arena);
    OffscreenBitmap offscreen_bitmap = offscreen_bitmap_create(&arena, window);

    EditorState editor_state;
    zero_memory(&editor_state, sizeof(EditorState));
    editor_settings_initialize_default(&editor_state.settings);

    while (platform_window_get_message(window)) {
        offscreen_bitmap_resize_synced(offscreen_bitmap);

        bitmap_clear(offscreen_bitmap_get_bitmap(offscreen_bitmap), editor_state.settings.color_scheme.background);

        offscreen_bitmap_swap(offscreen_bitmap);
    }

    offscreen_bitmap_destroy(&offscreen_bitmap);
    platform_window_destroy(&window);

    core_linear_arena_destroy(&arena);
    return 0;
}

#if OCEAN_PLATFORM_WINDOWS
#    include <Platform/Windows/WindowsEntryPoint.h>
#endif // OCEAN_PLATFORM_WINDOWS
