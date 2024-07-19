/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#include <Core/Memory/Arena.h>
#include <Core/Memory/MemoryOperations.h>
#include <Platform/Memory.h>

void core_linear_arena_initialize(LinearArena* arena, usize byte_count)
{
    if (arena->byte_count > 0)
        core_linear_arena_destroy(arena);

    arena->byte_count = byte_count;
    arena->memory = platform_memory_allocate(arena->byte_count);
    arena->offset = 0;

    core_linear_arena_reset(arena);
}

void core_linear_arena_destroy(LinearArena* arena)
{
    platform_memory_release(arena->memory, arena->byte_count);
    arena->memory = nullptr;
    arena->byte_count = 0;
    arena->offset = 0;
}

void core_linear_arena_reset(LinearArena* arena)
{
    arena->offset = 0;

    // Zero the memory that the arena will return as allocated.
    // This ensures that the arena always returns zero-initialized memory blocks.
    zero_memory(arena->memory, arena->byte_count);
}

void* core_linear_arena_allocate(LinearArena* arena, usize allocation_byte_count)
{
    constexpr usize allocation_alignment = 8;
    const usize byte_count_after_alignment = arena->offset & (allocation_alignment - 1);
    if (byte_count_after_alignment > 0) {
        const usize alignment = allocation_alignment - byte_count_after_alignment;
        if (arena->offset + alignment > arena->byte_count)
            return nullptr;
        arena->offset += alignment;
    }

    if (arena->offset + allocation_byte_count > arena->byte_count)
        return nullptr;
    void* memory_block = arena->memory + arena->offset;
    arena->offset += allocation_byte_count;
    return memory_block;
}
