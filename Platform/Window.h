/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#pragma once

#include <Core/Types.h>
#include <Core/Forward.h>

namespace Ocean {

// Opaque handle to a platform-specific window object.
using Window = void*;

Window platform_window_create(LinearArena& arena);
void platform_window_destroy(Window* window);

bool platform_window_get_message(Window window);

u32 platform_window_width(Window window);
u32 platform_window_height(Window window);
i32 platform_window_position_x(Window window);
i32 platform_window_position_y(Window window);

} // namespace Ocean
