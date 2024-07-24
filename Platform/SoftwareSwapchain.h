/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#pragma once

#include <Core/Types.h>
#include <Platform/Window.h>

void platform_software_swapchain_swap(Window window, u32 graphics_bitmap_width, u32 graphics_bitmap_height, void* graphics_bitmap_data);
