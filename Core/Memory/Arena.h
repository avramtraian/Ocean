/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#pragma once

#include <Core/Types.h>

struct LinearArena {
    ReadWriteBytes memory;
    usize byte_count;
    usize offset;
};

void core_linear_arena_initialize(LinearArena& arena, usize byte_count);
void core_linear_arena_destroy(LinearArena& arena);
void core_linear_arena_reset(LinearArena& arena);

void* core_linear_arena_allocate(LinearArena& arena, usize allocation_byte_count);
