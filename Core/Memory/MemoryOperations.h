/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#pragma once

#include <Core/Types.h>

void copy_memory(void* destination, const void* source, usize byte_count);
void set_memory(void* destination, ReadonlyByte byte_value, usize byte_count);
void zero_memory(void* destination, usize byte_count);
