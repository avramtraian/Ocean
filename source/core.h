/**
 * Copyright (c) 2025 Traian Avram. All rights reserved.
 * This source file is part of the Ocean text editor.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE(traian): Core types and definitions.
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
    #define OCEAN_CONFIGURATION_DEBUG   1
    #define OCEAN_CONFIGURATION_RELEASE 0
#else
    #define OCEAN_CONFIGURATION_DEBUG   0
    #define OCEAN_CONFIGURATION_RELEASE 1
#endif // _DEBUG

#define internal         static
#define local_persistent static
#define global_variable  static
#define function

#define NODISCARD           [[nodiscard]]
#define FUNCTION            __FUNCSIG__
#define FORCEINLINE         __forceinline
#define PLATFORM_DEBUGBREAK __debugbreak()
#define NULL                (0)

#define ASSERT(...)        if (!(__VA_ARGS__)) { PLATFORM_DEBUGBREAK; }
#define ASSERT_NOT_REACHED { PLATFORM_DEBUGBREAK; }
#define ARRAY_COUNT(x)     (sizeof(x) / sizeof((x)[0]))

using u8  = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long long;

using s8  = signed char;
using s16 = signed short;
using s32 = signed int;
using s64 = signed long long;

using usize   = unsigned long long;
using ssize   = signed long long;
using uintptr = unsigned long long;

using f32 = float;
using f64 = double;

using b8  = bool;
using b32 = int;

#define INVALID_SIZE ((usize)-1)

#define KILOBYTES(x) ((usize)1024 * (usize)(x))
#define MEGABYTES(x) ((usize)1024 * KILOBYTES(x))
#define GIGABYTES(x) ((usize)1024 * MEGABYTES(x))

////////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE(traian): Generic platform layer.
////////////////////////////////////////////////////////////////////////////////////////////////////

enum AllocateMemoryFlagsEnum : u8
{
    ALLOCATE_MEMORY_FLAG_RESERVE  = 1 << 0,
    ALLOCATE_MEMORY_FLAG_COMMIT   = 1 << 1,
    ALLOCATE_MEMORY_FLAG_ALLOCATE = ALLOCATE_MEMORY_FLAG_RESERVE | ALLOCATE_MEMORY_FLAG_COMMIT,
};
typedef u8 AllocateMemoryFlags;

#define PLATFORM_API_SIG_ALLOCATE_MEMORY(name) void * name(void *base_address, usize allocation_size, AllocateMemoryFlags flags)
PLATFORM_API_SIG_ALLOCATE_MEMORY(platform_allocate_memory);

enum ReleaseMemoryFlagsEnum : u8
{
    RELEASE_MEMORY_FLAG_RELEASE  = 1 << 0,
    RELEASE_MEMORY_FLAG_DECOMMIT = 1 << 1,
};
typedef u8 ReleaseMemoryFlags;

#define PLATFORM_API_SIG_RELEASE_MEMORY(name) void name(void *base_address, usize allocation_size, ReleaseMemoryFlags flags)
PLATFORM_API_SIG_RELEASE_MEMORY(platform_release_memory);

struct FileReadResult
{
    b8    is_valid;
    u64   file_size;
    void *file_data;
};

#define PLATFORM_API_SIG_GET_FILE_SIZE(name) u64 name(const char *filename);
PLATFORM_API_SIG_GET_FILE_SIZE(platform_get_file_size);

#define PLATFORM_API_SIG_READ_ENTIRE_FILE(name) FileReadResult name(const char *filename)
PLATFORM_API_SIG_READ_ENTIRE_FILE(platform_read_entire_file);

#define PLATFORM_API_SIG_RELEASE_FILE_READ_RESULT(name) void name(FileReadResult *result)
PLATFORM_API_SIG_RELEASE_FILE_READ_RESULT(platform_release_file_read_result);

#define PLATFORM_API_SIG_READ_ENTIRE_FILE_TO_BUFFER(name) FileReadResult name(const char *filename, void *buffer, usize buffer_size)
PLATFORM_API_SIG_READ_ENTIRE_FILE_TO_BUFFER(platform_read_entire_file_to_buffer);

#define PLATFORM_API_SIG_READ_ENTIRE_FILE_TO_ARENA(name) FileReadResult name(const char *filename, struct MemoryArena *arena)
PLATFORM_API_SIG_READ_ENTIRE_FILE_TO_ARENA(platform_read_entire_file_to_arena);

#define PLATFORM_API_SIG_WRITE_ENTIRE_FILE(name) u64 name(const char *filename, const void *buffer, usize buffer_size)
PLATFORM_API_SIG_WRITE_ENTIRE_FILE(platform_write_entire_file);

////////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE(traian): Math.
////////////////////////////////////////////////////////////////////////////////////////////////////

function inline s32
min_s32(s32 a, s32 b)
{
    const s32 result = (a < b) ? a : b;
    return result;
}

function inline s32
max_s32(s32 a, s32 b)
{
    const s32 result = (a > b) ? a : b;
    return result;
}

function inline s32
clamp_s32(s32 value, s32 min_bound, s32 max_bound)
{
    const s32 result = min_s32(max_s32(value, min_bound), max_bound);
    return result;
}

function inline u32
min_u32(u32 a, u32 b)
{
    const u32 result = (a < b) ? a : b;
    return result;
}

function inline u32
max_u32(u32 a, u32 b)
{
    const u32 result = (a > b) ? a : b;
    return result;
}

function inline u32
clamp_u32(u32 value, u32 min_bound, u32 max_bound)
{
    const u32 result = min_u32(max_u32(value, min_bound), max_bound);
    return result;
}

function inline u32
required_to_fill_u32(u32 step, u32 total_value)
{
    if (step == 0)
        return 0;

    u32 step_count = total_value / step;
    if (total_value % step != 0)
        ++step_count;
    return step_count;
}

struct Rect2s
{
    s32 min_x;
    s32 min_y;
    s32 max_x;
    s32 max_y;
};

function inline Rect2s
rect2s(s32 min_x, s32 min_y, s32 max_x, s32 max_y)
{
    const Rect2s result = { min_x, min_y, max_x, max_y };
    return result;
}

function inline b8
rect2_is_degenerated(Rect2s rect)
{
    const b8 result = (rect.min_x >= rect.max_x) || (rect.min_y >= rect.max_y);
    return result;
}

function inline void
rect2_size(Rect2s rect, u32 *out_size_x, u32 *out_size_y)
{
    if (rect2_is_degenerated(rect)) {
        *out_size_x = 0;
        *out_size_y = 0;
        return;
    }

    *out_size_x = rect.max_x - rect.min_x;
    *out_size_y = rect.max_y - rect.min_y;
}

function inline Rect2s
rect2_intersect(Rect2s a, Rect2s b)
{
    // NOTE(traian): Return a degenerated rectangle if the intersection is void.
    Rect2s result = {};

    s32 min_x = max_s32(a.min_x, b.min_x);
    s32 max_x = min_s32(a.max_x, b.max_x);
    if (min_x >= max_x)
        return result;

    s32 min_y = max_s32(a.min_y, b.min_y);
    s32 max_y = min_s32(a.max_y, b.max_y);
    if (min_y >= max_y)
        return result;

    result.min_x = min_x;
    result.min_y = min_y;
    result.max_x = max_x;
    result.max_y = max_y;
    return result;
}

struct LinearColor
{
    u8 b;
    u8 g;
    u8 r;
    u8 a;
};

function inline LinearColor
linear_color(u8 r, u8 g, u8 b, u8 a = 255)
{
    const LinearColor result = { b, g, r, a };
    return result;
}

function inline u32
linear_color_pack_to_u32(LinearColor unpacked_color)
{
    const u32 packed = *(u32 *)&unpacked_color;
    return packed;
}

function inline LinearColor
linear_color_unpack_from_u32(u32 packed_color)
{
    const LinearColor unpacked = *(LinearColor *)&packed_color;
    return unpacked;
}

function inline LinearColor
linear_color_blend(LinearColor a, LinearColor b, f32 alpha)
{
    const f32 inv_alpha = 1.0F - alpha;
    LinearColor blended;
    blended.b = (u8)((f32)a.b * inv_alpha + (f32)b.b * alpha);
    blended.g = (u8)((f32)a.g * inv_alpha + (f32)b.g * alpha);
    blended.r = (u8)((f32)a.r * inv_alpha + (f32)b.r * alpha);
    blended.a = (u8)((f32)a.a * inv_alpha + (f32)b.a * alpha);
    return blended;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE(traian): Memory.
////////////////////////////////////////////////////////////////////////////////////////////////////

function void copy_memory(void* destination, const void* source, usize size);
function void set_memory(void* destination, u8 byte_value, usize size);
function void zero_memory(void* destination, usize size);

struct MemoryArena
{
    void *data;
    usize committed_size;
    usize reserved_size;
    usize allocated;
};

function void memory_arena_initialize(MemoryArena *arena, usize arena_initial_size, usize arena_max_size = 0);
function void memory_arena_destroy(MemoryArena *arena);
function void memory_arena_reset(MemoryArena *arena);
function void * memory_arena_allocate(MemoryArena *arena, usize allocation_size);

#define ARENA_PUSH_STRUCT(arena, struct_type) (struct_type *)memory_arena_allocate(arena, sizeof(struct_type))
#define ARENA_PUSH_ARRAY(arena, struct_type, count) (struct_type *)memory_arena_allocate(arena, (count) * sizeof(struct_type))
#define ARENA_PUSH_COPY(arena, buffer, buffer_size) { void *_dst = memory_arena_allocate(arena, buffer_size); copy_memory(_dst, buffer, buffer_size); }

////////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE(traian): String library.
////////////////////////////////////////////////////////////////////////////////////////////////////

struct String
{
    char *characters;
    usize byte_count;
};

function String string_initialize(char *characters, usize byte_count);
function String string_allocate(MemoryArena *arena, usize byte_count);

// NOTE: The returned string is immutable, but the API doesn't specify that in any way.
#define STRING_FROM_LITERAL(literal) string_initialize((char *)(literal), sizeof(literal) - sizeof('\0'))

enum NumericBaseEnum : u8
{
    NUMERIC_BASE_DECIMAL,
    NUMERIC_BASE_HEX,
    NUMERIC_BASE_OCT,
    NUMERIC_BASE_BINARY,
};
using NumericBase = u8;

function usize string_size_from_uint(u64 value, NumericBase numeric_base);
function String string_from_uint(MemoryArena *arena, u64 value, NumericBase numeric_base);

struct Utf8DecodeResult
{
    b8 is_valid;
    u32 codepoint;
    usize byte_count;
};

function Utf8DecodeResult utf8_decode_byte_sequence(u8 *bytes, usize byte_count);
