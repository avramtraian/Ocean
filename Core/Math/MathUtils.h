/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#pragma once

#include <Core/Assertion.h>
#include <Core/Types.h>

inline u32 min_u32(u32 a, u32 b)
{
    return (a < b) ? a : b;
}

inline u32 max_u32(u32 a, u32 b)
{
    return (a > b) ? a : b;
}

inline i32 min_i32(i32 a, i32 b)
{
    return (a < b) ? a : b;
}

inline i32 max_i32(i32 a, i32 b)
{
    return (a > b) ? a : b;
}

inline u32 clamp_u32(u32 value, u32 min_bound, u32 max_bound)
{
    VERIFY(min_bound <= max_bound);
    return min_u32(max_u32(value, min_bound), max_bound);
}

inline i32 clamp_i32(i32 value, i32 min_bound, i32 max_bound)
{
    VERIFY(min_bound <= max_bound);
    return min_i32(max_i32(value, min_bound), max_bound);
}
