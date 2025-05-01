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

internal u8
string_get_numeric_base_value(NumericBase numeric_base)
{
    switch (numeric_base) {
        case NUMERIC_BASE_DECIMAL: return 10;
        case NUMERIC_BASE_HEX:     return 16;
        case NUMERIC_BASE_OCT:     return 8;
        case NUMERIC_BASE_BINARY:  return 2;
    }

    // NOTE: Default to decimal.
    return 10;
}

function usize
string_size_from_uint(u64 value, NumericBase numeric_base)
{
    if (value == 0)
        return sizeof(char);

    const u8 base_value = string_get_numeric_base_value(numeric_base);
    usize string_size = 0;
    while (value != 0) {
        string_size += sizeof(char);
        value /= base_value;
    }

    return string_size;
}

function String
string_from_uint(MemoryArena *arena, u64 value, NumericBase numeric_base)
{
    const usize string_size = string_size_from_uint(value, numeric_base);
    String string = string_allocate(arena, string_size);

    if (value == 0) {
        ASSERT(string.byte_count == sizeof(char));
        string.characters[0] = '0';
        return string;
    }

    const char *digit_table_decimal = "0123456789";
    const char *digit_table_hex     = "0123456789ABCDEF";
    const char *digit_table_oct     = "01234567";
    const char *digit_table_binary  = "01";

    const char* digit_table = NULL;
    switch (numeric_base) {
        case NUMERIC_BASE_DECIMAL: digit_table = digit_table_decimal; break;
        case NUMERIC_BASE_HEX:     digit_table = digit_table_hex;     break;
        case NUMERIC_BASE_OCT:     digit_table = digit_table_oct;     break;
        case NUMERIC_BASE_BINARY:  digit_table = digit_table_binary;  break;
    }

    const u8 base_value = string_get_numeric_base_value(numeric_base);
    usize byte_offset = string_size - 1;

    while (value != 0) {
        const u8 digit_index = value % base_value;
        value /= base_value;

        string.characters[byte_offset--] = digit_table[digit_index];
    }

    return string;
}

function Utf8DecodeResult
utf8_decode_byte_sequence(u8 *bytes, usize byte_count)
{
    Utf8DecodeResult result = {};
    result.is_valid = false;
    result.codepoint = 0;
    result.byte_count = 0;

    if (byte_count == 0)
        return result;

    if ((bytes[0] & 0x80) == 0x00) {
        result.is_valid = true;
        result.codepoint = bytes[0];
        result.byte_count = 1;

        return result;
    }

    if ((bytes[0] & 0xE0) == 0xC0) {
        if (byte_count < 2)
            return result;

        result.is_valid = true;
        result.codepoint += (bytes[0] & 0x1F) << 6;
        result.codepoint += (bytes[1] & 0x3F) << 0;
        result.byte_count = 2;

        return result;
    }

    if ((bytes[0] & 0xF0) == 0xE0) {
        if (byte_count < 3)
            return result;

        result.is_valid = true;
        result.codepoint += (bytes[0] & 0x1F) << 12;
        result.codepoint += (bytes[1] & 0x3F) << 6;
        result.codepoint += (bytes[2] & 0x3F) << 0;
        result.byte_count = 3;

        return result;
    }

    if ((bytes[0] & 0xF8) == 0xF0) {
        if (byte_count < 4)
            return result;

        result.is_valid = true;
        result.codepoint += (bytes[0] & 0x1F) << 18;
        result.codepoint += (bytes[1] & 0x3F) << 12;
        result.codepoint += (bytes[2] & 0x3F) << 6;
        result.codepoint += (bytes[3] & 0x3F) << 0;
        result.byte_count = 4;

        return result;
    }

    return result;
}
