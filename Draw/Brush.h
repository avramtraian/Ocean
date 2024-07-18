/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#pragma once

#include <Core/Math/Color.h>
#include <Core/Math/Rect.h>
#include <Core/Memory/Arena.h>
#include <Draw/Bitmap.h>

// Opaque handle towards a brush.
typedef void* Brush;

Brush brush_create(LinearArena* arena);

void brush_destroy(Brush* brush_handle);

// NOTE: The alpha component of the provided linear color will be ignored (considered 255).
void brush_draw_opaque_quad(Brush brush_handle, Bitmap bitmap_handle, Rect quad, LinearColor color);

// NOTE: The alpha component of every pixel to draw will be ignored (considered 255).
//       The provided destinaton bitmap and the bitmap to draw must have compatible formats.
void brush_draw_opaque_bitmap(Brush brush_handle, Bitmap bitmap_handle, Bitmap bitmap_to_draw_handle, Vector2i offset);

void brush_draw_font_bitmap(Brush brush_handle, Bitmap bitmap_handle, Bitmap font_bitmap_handle, Vector2i offset, LinearColor font_color);
