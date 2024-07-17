/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#pragma once

#include <Core/Math/Vector.h>

typedef struct Rect {
    Vector2i offset;
    Vector2u extent;
} Rect;

inline Rect rect(i32 x, i32 y, u32 width, u32 height)
{
    Rect r;
    r.offset = vec2i(x, y);
    r.extent = vec2u(width, height);
    return r;
}
