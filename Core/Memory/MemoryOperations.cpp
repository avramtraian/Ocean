/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#include <Core/Memory/MemoryOperations.h>

namespace Ocean {

void copy_memory(void* destination, const void* source, usize byte_count)
{
    const WriteonlyBytes dst_buffer = static_cast<WriteonlyBytes>(destination);
    const ReadonlyBytes src_buffer = static_cast<ReadonlyBytes>(source);

    for (usize byte_offset = 0; byte_offset < byte_count; ++byte_offset)
        dst_buffer[byte_offset] = src_buffer[byte_offset];
}

void set_memory(void* destination, ReadonlyByte byte_value, usize byte_count)
{
    const WriteonlyBytes dst_buffer = static_cast<WriteonlyBytes>(destination);

    for (usize byte_offset = 0; byte_offset < byte_count; ++byte_offset)
        dst_buffer[byte_offset] = byte_value;
}

void zero_memory(void* destination, usize byte_count)
{
    // NOTE: Wrap around the 'set_memory' function.
    set_memory(destination, 0, byte_count);
}

} // namespace Ocean
