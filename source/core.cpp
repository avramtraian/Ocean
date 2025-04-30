/**
 * Copyright (c) 2025 Traian Avram. All rights reserved.
 * This source file is part of the Ocean text editor.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#include "core.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE(traian): Memory.
////////////////////////////////////////////////////////////////////////////////////////////////////

function void
copy_memory(void *destination, const void *source, usize size)
{
    u8 *dst = (u8 *)destination;
    const u8 *src = (const u8 *)source;

    for (usize offset = 0; offset < size; ++offset)
        dst[offset] = src[offset];
}

function void
set_memory(void *destination, u8 byte_value, usize size)
{
    u8 *dst = (u8 *)destination;

    for (usize offset = 0; offset < size; ++offset)
        dst[offset] = byte_value;
}

function void
zero_memory(void *destination, usize size)
{
    u8 *dst = (u8 *)destination;

    for (usize offset = 0; offset < size; ++offset)
        dst[offset] = 0;
}

function void
memory_arena_initialize(MemoryArena *arena, usize arena_initial_size, usize arena_max_size /*= 0*/)
{
    if (arena == NULL)
        return;

    if (arena_max_size == 0)
        arena_max_size = arena_initial_size;

    arena->data = platform_allocate_memory(NULL, arena_max_size, ALLOCATE_MEMORY_FLAG_RESERVE);
    platform_allocate_memory(arena->data, arena_initial_size, ALLOCATE_MEMORY_FLAG_COMMIT);

    arena->committed_size = arena_initial_size;
    arena->reserved_size = arena_max_size;
    arena->allocated = 0;
}

function void
memory_arena_destroy(MemoryArena *arena)
{
    if (arena == NULL)
        return;

    platform_release_memory(arena->data, 0, RELEASE_MEMORY_FLAG_RELEASE);
    arena->data = NULL;
    arena->committed_size = 0;
    arena->reserved_size = 0;
    arena->allocated = 0;
}

function void
memory_arena_reset(MemoryArena *arena)
{
    if (arena == NULL)
        return;

    zero_memory(arena->data, arena->allocated);
    arena->allocated = 0;
}

function void *
memory_arena_allocate(MemoryArena *arena, usize allocation_size)
{
    if (arena == NULL || allocation_size == 0)
        return NULL;

    ASSERT(arena->allocated + allocation_size <= arena->reserved_size);

    if (arena->allocated + allocation_size > arena->committed_size) {
        usize expansion_size = (arena->committed_size > 0) ? arena->committed_size : KILOBYTES(256);
        if (expansion_size < allocation_size)
            expansion_size = allocation_size;
        if (arena->committed_size + expansion_size > arena->reserved_size)
            expansion_size = arena->reserved_size - arena->committed_size;
        
        platform_allocate_memory((u8 *)arena->data + arena->committed_size, expansion_size, ALLOCATE_MEMORY_FLAG_COMMIT);
        arena->committed_size += expansion_size;
    }

    void *allocation_block = (u8 *)arena->data + arena->allocated;
    arena->allocated += allocation_size;
    return allocation_block;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE(traian): String library.
////////////////////////////////////////////////////////////////////////////////////////////////////

function String
string_initialize(char *characters, usize byte_count)
{
    String result;
    result.characters = characters;
    result.byte_count = byte_count;
    return result;
}

function String
string_allocate(MemoryArena *arena, usize byte_count)
{
    String result;
    result.characters = (char *)memory_arena_allocate(arena, byte_count);
    result.byte_count = byte_count;
    return result;
}

