/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#pragma once

#include <Core/Types.h>

// Reserved for large allocations. Use an allocator implemented in the Core module for general purpose allocations.
ReadWriteBytes platform_memory_allocate(usize byte_count);
void platform_memory_release(void* memory_block, usize byte_count);
