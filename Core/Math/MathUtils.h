/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#pragma once

#include <Core/Assertion.h>
#include <Core/Types.h>

#define OCEAN_MIN_FUNCTION_IMPLEMENTATION(type) \
    inline type min_##type(type a, type b)      \
    {                                           \
        return (a < b) ? a : b;                 \
    }

#define OCEAN_MAX_FUNCTION_IMPLEMENTATION(type) \
    inline type max_##type(type a, type b)      \
    {                                           \
        return (a > b) ? a : b;                 \
    }

#define OCEAN_CLAMP_FUNCTION_IMPLEMENTATION(type)                        \
    inline type clamp_##type(type value, type min_bound, type max_bound) \
    {                                                                    \
        VERIFY(min_bound <= max_bound);                                  \
        return min_##type(max_##type(value, min_bound), max_bound);      \
    }

#define OCEAN_DEFINE_MATH_FUNCTIONS_FOR_EACH_TYPE(function_macro) \
    function_macro(u8);                                           \
    function_macro(u16);                                          \
    function_macro(u32);                                          \
    function_macro(u64);                                          \
    function_macro(i8);                                           \
    function_macro(i16);                                          \
    function_macro(i32);                                          \
    function_macro(i64);                                          \
    function_macro(float);                                        \
    function_macro(double);                                       \
    function_macro(usize);                                        \
    function_macro(ssize);

OCEAN_DEFINE_MATH_FUNCTIONS_FOR_EACH_TYPE(OCEAN_MIN_FUNCTION_IMPLEMENTATION)
OCEAN_DEFINE_MATH_FUNCTIONS_FOR_EACH_TYPE(OCEAN_MAX_FUNCTION_IMPLEMENTATION)
OCEAN_DEFINE_MATH_FUNCTIONS_FOR_EACH_TYPE(OCEAN_CLAMP_FUNCTION_IMPLEMENTATION)

#undef OCEAN_DEFINE_MATH_FUNCTIONS_FOR_EACH_TYPE

#undef OCEAN_MIN_FUNCTION_IMPLEMENTATION
#undef OCEAN_MAX_FUNCTION_IMPLEMENTATION
#undef OCEAN_CLAMP_FUNCTION_IMPLEMENTATION
