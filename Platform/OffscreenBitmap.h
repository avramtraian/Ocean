/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#pragma once

#include <Draw/Bitmap.h>
#include <Platform/Window.h>

typedef struct OffscreenBitmap {
    Bitmap bitmap;
    Window window;
    void* internal_data;
} OffscreenBitmap;

void platform_offscreen_bitmap_create(OffscreenBitmap* offscreen_bitmap, Window window);

void platform_offscreen_bitmap_destroy(OffscreenBitmap* offscreen_bitmap);

void platform_offscreen_bitmap_draw(const OffscreenBitmap* offscreen_bitmap);

// Resizes the offscreen bitmap to the parent window's current size.
// If the dimensions are already matching, no action is performed.
void platform_offscreen_bitmap_resize_synced(OffscreenBitmap* offscreen_bitmap);
