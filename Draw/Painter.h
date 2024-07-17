/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#pragma once

#include <Core/Math/Color.h>
#include <Core/Math/Rect.h>
#include <Draw/Forward.h>

void painter_draw_quad(Bitmap* bitmap, Rect quad, LinearColor color);

// NOTE: The alpha component of the provided linear color will be ignored (and considered 255).
//       As no color blending will be performed, this function is significantly faster than
//       the generic version, and thus should be prefered whenever possible.
void painter_draw_opaque_quad(Bitmap* bitmap, Rect quad, LinearColor color);

// NOTE: The quad must fit entirely on the bitmap, as no clipping is allowed. Slightly faster.
void painter_draw_full_quad(Bitmap* bitmap, Rect quad, LinearColor color);

// NOTE: The quad must fit entirely on the bitmap, as no clipping is allowed. Slightly faster.
//       See 'painter_draw_opaque_quad' for reference about the opaque color.
void painter_draw_full_opaque_quad(Bitmap* bitmap, Rect quad, LinearColor color);
