/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#pragma once

#include <Core/Types.h>

typedef struct Vector2u {
    u32 x;
    u32 y;
} Vector2u;

typedef struct Vector2i {
    i32 x;
    i32 y;
} Vector2i;

inline Vector2u vec2u(u32 x, u32 y)
{
    Vector2u vector;
    vector.x = x;
    vector.y = y;
    return vector;
}

inline Vector2i vec2i(i32 x, i32 y)
{
    Vector2i vector;
    vector.x = x;
    vector.y = y;
    return vector;
}
