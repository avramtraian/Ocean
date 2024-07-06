/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#include <Core/Memory/Arena.h>
#include <Core/Types.h>
#include <Platform/Window.h>

static i32 guarded_main()
{
    LinearArena arena = {};
    core_linear_arena_initialize(arena, 16 * MiB);

    Window window = platform_window_create(arena);
    while (platform_window_get_message(window)) {}
    platform_window_destroy(&window);

    core_linear_arena_destroy(arena);
    return 0;
}

#if OCEAN_PLATFORM_WINDOWS
#    include <Platform/Windows/WindowsEntryPoint.h>
#endif // OCEAN_PLATFORM_WINDOWS
