/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#pragma once

#include <Core/Memory/Arena.h>
#include <Platform/Window.h>

// Opaque handle towards an offscreen bitmap.
typedef void* OffscreenBitmap;

OffscreenBitmap offscreen_bitmap_create(LinearArena* arena, Window window);

void offscreen_bitmap_destroy(OffscreenBitmap* offscreen_bitmap_handle);

void offscreen_bitmap_swap(OffscreenBitmap offscreen_bitmap_handle);

void offscreen_bitmap_resize_synced(OffscreenBitmap offscreen_bitmap_handle);

Bitmap offscreen_bitmap_get_bitmap(OffscreenBitmap offscreen_bitmap_handle);
