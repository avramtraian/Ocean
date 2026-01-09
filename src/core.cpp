/*
 * Copyright (c) 2025-2026 Traian Avram. All rights reserved.
 * This file is part of my personal text editor and is distributed under the MIT license.
 */

#define KiB(x) (1024 * (x))
#define MiB(x) (1024 * KiB(x))
#define GiB(x) (1024 * MiB(x))

function void
zero_memory(void* destination, usize size)
{
    // @Speed: Intrinsics!
    u8* dst = (u8*)destination;
    u8* dst_end = dst + size;
    while (dst != dst_end) {
        *dst = 0;
        ++dst;
    }
}

function void
copy_memory(void* destination, void* source, usize size)
{
    // @Speed: Intrinsics!
    u8* dst = (u8*)destination;
    u8* src = (u8*)source;
    u8* dst_end = dst + size;
    while (dst != dst_end) {
        *dst = *src;
        ++dst;
        ++src;
    }
}

#define ZERO_STRUCT(x)          zero_memory(&(x), sizeof(x))
#define ZERO_STRUCT_POINTER(x)  zero_memory(x, sizeof(*(x)))
#define ZERO_STRUCT_ARRAY(x, c) zero_memory(x, (c) * sizeof((x)[0]))

struct MemoryArena {
    u8* data;
    usize used;
    usize committed;
    usize reserved;
};

function MemoryArena create_arena(usize committed, usize reserved)
{
    ASSERT(committed <= reserved);
    committed = os_get_memory_page_aligned(committed);
    reserved = os_get_memory_page_aligned(reserved);

    MemoryArena arena = {};
    arena.committed = committed;
    arena.reserved = reserved;
    arena.data = (u8*)VirtualAlloc(NULL, reserved, MEM_RESERVE, PAGE_READWRITE);
    VirtualAlloc(arena.data, committed, MEM_COMMIT, PAGE_READWRITE);
    return arena;
}

function void
reset_memory_arena(MemoryArena* arena)
{
    zero_memory(arena->data, arena->used);
    arena->used = 0;
}

function void*
allocate_from_arena(MemoryArena* arena, usize size, usize alignment)
{
    uintptr base_address = (uintptr)(arena->data + arena->used);
    uintptr aligned_address = align_to_pow2(base_address, alignment); // Must be a power of 2.
    usize alignment_offset = aligned_address - base_address;
    usize total_size = alignment_offset + size;

    ASSERT(arena->used + total_size <= arena->reserved);
    if (arena->used + total_size > arena->committed) {
        usize new_committed = 2 * arena->committed;
        if (arena->used + total_size > new_committed)
            new_committed = os_get_memory_page_aligned(arena->used + total_size);

        VirtualAlloc(arena->data + arena->committed, new_committed - arena->committed, MEM_COMMIT, PAGE_READWRITE);
        arena->committed = new_committed;
    }

    arena->used += total_size;
    void* allocation_address = (void*)aligned_address;
    zero_memory(allocation_address, size);
    return allocation_address;
}

#define PUSH_STRUCT(arena, StructName)       (StructName*)allocate_from_arena(arena, sizeof(StructName), alignof(StructName))
#define PUSH_ARRAY(arena, StructName, count) (StructName*)allocate_from_arena(arena, (count) * sizeof(StructName), alignof(StructName))

struct GlobalArenas {
    MemoryArena* eternal;
    MemoryArena* frame;
    MemoryArena* fonts;
};

internal GlobalArenas g_arenas;

struct TemporaryMemory {
    MemoryArena* arena;
    usize base_used;
};

function TemporaryMemory
begin_temporary_memory(MemoryArena* arena)
{
    TemporaryMemory result = {};
    result.arena = arena;
    result.base_used = arena->used;
}

function void
end_temporary_memory(TemporaryMemory* temporary_memory)
{
    if (temporary_memory && temporary_memory->arena) {
        ASSERT(temporary_memory->arena->used >= temporary_memory->base_used);
        zero_memory(temporary_memory->arena->data + temporary_memory->base_used,
                    temporary_memory->arena->used - temporary_memory->base_used);
        temporary_memory->arena->used = temporary_memory->base_used;
    }

    ZERO_STRUCT_POINTER(temporary_memory);
}

struct String {
    u8* data;
    usize size;
    bool owns_memory;
};

internal String
create_string(u8* data, usize size, bool owns_memory)
{
    String result;
    result.data = data;
    result.size = size;
    result.owns_memory = owns_memory;
    return result;
}

internal void
free(String* string)
{
    // @Leak!
    ZERO_STRUCT_POINTER(string);
}

#define STRING_LIT(x) (create_string((u8*)(x), sizeof(x) - sizeof('\0'), false))
