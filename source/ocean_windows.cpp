// Copyright (c) 2025 Traian Avram. All rights reserved.

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define internal         static
#define local_persistent static
#define global_variable  static
#define function

#define NODISCARD           [[nodiscard]]
#define FUNCTION            __FUNCSIG__
#define FORCEINLINE         __forceinline
#define PLATFORM_DEBUGBREAK __debugbreak()

#define ASSERT(...)        if (!(__VA_ARGS__)) { PLATFORM_DEBUGBREAK; }
#define ASSERT_NOT_REACHED { PLATFORM_DEBUGBREAK; }

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long long s64;
typedef unsigned long long usize;
typedef signed long long ssize;
typedef unsigned long long uintptr;
typedef float f32;
typedef double f64;
typedef bool b8;
typedef int b32;

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

internal inline s32
min_s32(s32 a, s32 b)
{
    const s32 result = (a < b) ? a : b;
    return result;
}

internal inline s32
max_s32(s32 a, s32 b)
{
    const s32 result = (a > b) ? a : b;
    return result;
}

internal inline s32
clamp_s32(s32 value, s32 min_bound, s32 max_bound)
{
    const s32 result = min_s32(max_s32(value, min_bound), max_bound);
    return result;
}

internal inline u32
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

internal inline Rect2s
rect2s(s32 min_x, s32 min_y, s32 max_x, s32 max_y)
{
    const Rect2s result = { min_x, min_y, max_x, max_y };
    return result;
}

internal inline b8
rect2_is_degenerated(Rect2s rect)
{
    const b8 result = (rect.min_x >= rect.max_x) || (rect.min_y >= rect.max_y);
    return result;
}

internal inline void
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

internal inline Rect2s
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

internal inline LinearColor
linear_color(u8 r, u8 g, u8 b, u8 a = 255)
{
    const LinearColor result = { b, g, r, a };
    return result;
}

internal inline u32
linear_color_pack_to_u32(LinearColor unpacked_color)
{
    const u32 packed = *(u32 *)&unpacked_color;
    return packed;
}

internal inline LinearColor
linear_color_unpack_from_u32(u32 packed_color)
{
    const LinearColor unpacked = *(LinearColor *)&packed_color;
    return unpacked;
}

internal inline LinearColor
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

internal void
copy_memory(void *destination, const void *source, usize size)
{
    u8 *dst = (u8 *)destination;
    const u8 *src = (const u8 *)source;

    for (usize offset = 0; offset < size; ++offset)
        dst[offset] = src[offset];
}

internal void
set_memory(void *destination, u8 byte_value, usize size)
{
    u8 *dst = (u8 *)destination;

    for (usize offset = 0; offset < size; ++offset)
        dst[offset] = byte_value;
}

internal void
zero_memory(void *destination, usize size)
{
    u8 *dst = (u8 *)destination;

    for (usize offset = 0; offset < size; ++offset)
        dst[offset] = 0;
}

struct MemoryArena
{
    void *data;
    usize committed_size;
    usize reserved_size;
    usize allocated;
};

internal void
memory_arena_initialize(MemoryArena *arena, usize arena_initial_size, usize arena_max_size = 0)
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

internal void
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

internal void
memory_arena_reset(MemoryArena *arena)
{
    if (arena == NULL)
        return;

    zero_memory(arena->data, arena->allocated);
    arena->allocated = 0;
}

internal void *
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

#define ARENA_PUSH_STRUCT(arena, struct_type) (struct_type *)memory_arena_allocate(arena, sizeof(struct_type))
#define ARENA_PUSH_ARRAY(arena, struct_type, count) (struct_type *)memory_arena_allocate(arena, (count) * sizeof(struct_type))
#define ARENA_PUSH_COPY(arena, buffer, buffer_size) { void *_dst = memory_arena_allocate(arena, buffer_size); copy_memory(_dst, buffer, buffer_size); }

////////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE(traian): Graphics resources.
////////////////////////////////////////////////////////////////////////////////////////////////////

struct Bitmap
{
    void *pixels;
    u32   size_x;
    u32   size_y;
    u32   bytes_per_pixel;
};

internal inline usize
bitmap_get_stride(Bitmap *bitmap)
{
    if (bitmap == NULL)
        return 0;

    const usize result = (usize)bitmap->size_x * (usize)bitmap->bytes_per_pixel;
    return result;
}

internal inline usize
bitmap_get_pixels_buffer_size(Bitmap *bitmap)
{
    if (bitmap == NULL)
        return 0;

    const usize result = (usize)bitmap->size_y * bitmap_get_stride(bitmap);
    return result;
}

internal inline void *
bitmap_get_row_address(Bitmap *bitmap, u32 row_index)
{
    if (bitmap == NULL || row_index > bitmap->size_y)
        return NULL;

    const usize byte_offset = (usize)row_index * bitmap_get_stride(bitmap);
    return (u8 *)bitmap->pixels + byte_offset;
}

internal inline void *
bitmap_get_pixel_address(Bitmap *bitmap, u32 offset_x, u32 offset_y)
{
    if (bitmap == NULL || offset_x > bitmap->size_x || offset_y > bitmap->size_y)
        return NULL;

    const usize offset_in_row = (usize)offset_x * (usize)bitmap->bytes_per_pixel;
    return (u8 *)bitmap_get_row_address(bitmap, offset_y) + offset_in_row;
}

internal void
bitmap_initialize(Bitmap *bitmap, MemoryArena *arena, u32 size_x, u32 size_y, u32 bytes_per_pixel)
{
    if (bitmap == NULL)
        return;

    if (arena) {
        bitmap->size_x = size_x;
        bitmap->size_y = size_y;
        bitmap->bytes_per_pixel = bytes_per_pixel;
        bitmap->pixels = memory_arena_allocate(arena, bitmap_get_pixels_buffer_size(bitmap));
    }
    else {
        bitmap->pixels = NULL;
        bitmap->size_x = 0;
        bitmap->size_y = 0;
        bitmap->bytes_per_pixel = 0;
    }
}

#define ASCII_CHARACTER_FIRST ((u32)'!')
#define ASCII_CHARACTER_LAST  ((u32)'~')
#define ASCII_CHARACTER_COUNT (ASCII_CHARACTER_LAST - ASCII_CHARACTER_FIRST + 1)

struct FontGlyph
{
    Bitmap bitmap;
    u32    codepoint;
    s32    offset_x;
    s32    offset_y;
};

struct Font
{
    f32         height;
    u32         ascent;
    u32         descent;
    u32         line_gap;
    u32         advance;
    FontGlyph   ascii_glyphs[ASCII_CHARACTER_COUNT];
};

internal void
font_initialize(Font *font, MemoryArena *permanent_arena, MemoryArena *work_arena,
                void *ttf_buffer_data, usize ttf_buffer_size, f32 font_height)
{
    stbtt_fontinfo font_info;
    stbtt_InitFont(&font_info, (u8 *)ttf_buffer_data, stbtt_GetFontOffsetForIndex((u8 *)ttf_buffer_data, 0));
    f32 scale = stbtt_ScaleForPixelHeight(&font_info, font_height);

    int advance;
    stbtt_GetCodepointHMetrics(&font_info, 'X', &advance, NULL);
    font->advance = (u32)((f32)advance * scale);

    int ascent, descent, line_gap;
    stbtt_GetFontVMetrics(&font_info, &ascent, &descent, &line_gap);
    font->ascent = (u32)((f32)ascent * scale);
    font->descent = (u32)((f32)-descent * scale);
    font->line_gap = (u32)((f32)line_gap * scale);

    // NOTE(traian): Load the glyphs for all visible ASCII codepoints.
    for (u32 ascii_codepoint_index = 0; ascii_codepoint_index < ASCII_CHARACTER_COUNT; ++ascii_codepoint_index) {
        const u32 codepoint = ASCII_CHARACTER_FIRST + ascii_codepoint_index;
        FontGlyph *glyph = font->ascii_glyphs + ascii_codepoint_index;
        
        // TODO(traian): Figure it out how 'stbtt_GetCodepointBitmapBox' works and just rasterize the glyph
        // directly into our own bitmap (and flip it afterwards), instead of allocating temporary memory from the heap.
        // The current behaviour defeats the entire purpose of the work memory arena! - 30 Mar 2025
        int size_x, size_y, offset_x, offset_y;
        u8 *flipped_bitmap_data = stbtt_GetCodepointBitmap(&font_info, scale, scale, codepoint, &size_x, &size_y, &offset_x, &offset_y);

        bitmap_initialize(&glyph->bitmap, permanent_arena, size_x, size_y, 1);

        if (size_x > 0 && size_y > 0) {
            u8 *dst_row = (u8 *)bitmap_get_row_address(&glyph->bitmap, size_y - 1);
            u8 *src_row = flipped_bitmap_data;

            for (u32 y = 0; y < (u32)size_y; ++y) {
                for (u32 x = 0; x < (u32)size_x; ++x) {
                    dst_row[x] = src_row[x];
                }

                dst_row -= size_x;
                src_row += size_x;
            }
        }

        glyph->codepoint = codepoint;
        glyph->offset_x = offset_x;
        glyph->offset_y = -(size_y + offset_y);

        stbtt_FreeBitmap(flipped_bitmap_data, NULL);
    }
}

internal FontGlyph *
font_get_glyph(Font *font, u32 codepoint)
{
    if (ASCII_CHARACTER_FIRST <= codepoint && codepoint <= ASCII_CHARACTER_LAST) {
        const u32 glyph_index = codepoint - ASCII_CHARACTER_FIRST;
        return font->ascii_glyphs + glyph_index;
    }

    // TODO(traian): Support extended Unicode pages!
    return NULL;
}

struct TiledTextCell
{
    u32         codepoint;
    LinearColor color;
};

struct TiledTextBuffer
{
    u32            cell_count_x;
    u32            cell_count_y;
    TiledTextCell *cells;
    u32            cell_size_x;
    u32            cell_size_y;
    u32            line_spacing;
    s32            viewport_offset_x;
    s32            viewport_offset_y;
    u32            viewport_size_x;
    u32            viewport_size_y;
    u32            offset_x;
    u32            offset_y;
};

internal void
tiled_text_buffer_cell_count_from_viewport(u32 viewport_size_x, u32 viewport_size_y,
                                           u32 cell_size_x, u32 cell_size_y, u32 line_spacing, bool is_offset_allowed,
                                           u32 *out_cell_count_x, u32 *out_cell_count_y)
{
    u32 viewport_with_offset_x = viewport_size_x;
    u32 viewport_with_offset_y = viewport_size_y;
    if (is_offset_allowed) {
        viewport_with_offset_x += (cell_size_x - 1);
        viewport_with_offset_y += (cell_size_y - 1);
    }

    *out_cell_count_x = required_to_fill_u32(cell_size_x, viewport_with_offset_x);
    *out_cell_count_y = required_to_fill_u32(cell_size_y + line_spacing, viewport_with_offset_y + line_spacing);
}

internal void
tiled_text_buffer_initialize(TiledTextBuffer *buffer, MemoryArena *arena, u32 cell_count_x, u32 cell_count_y)
{
    if (buffer == NULL)
        return;

    buffer->cell_count_x = cell_count_x;
    buffer->cell_count_y = cell_count_y;
    buffer->cells = ARENA_PUSH_ARRAY(arena, TiledTextCell, cell_count_x * cell_count_y);
    buffer->cell_size_x = 0;
    buffer->cell_size_y = 0;
    buffer->viewport_offset_x = 0;
    buffer->viewport_offset_y = 0;
    buffer->viewport_size_x = 0;
    buffer->viewport_size_y = 0;
    buffer->offset_x = 0;
    buffer->offset_y = 0;
}

internal void
tiled_text_buffer_set_cell_size(TiledTextBuffer *buffer, u32 cell_size_x, u32 cell_size_y, u32 line_spacing)
{
    if (buffer == NULL)
        return;

    buffer->cell_size_x = cell_size_x;
    buffer->cell_size_y = cell_size_y;
    buffer->line_spacing = line_spacing;
}

internal void
tiled_text_buffer_set_viewport(TiledTextBuffer *buffer, u32 viewport_offset_x, u32 viewport_offset_y,
                               u32 viewport_size_x, u32 viewport_size_y)
{
    if (buffer == NULL)
        return;

    buffer->viewport_offset_x = viewport_offset_x;
    buffer->viewport_offset_y = viewport_offset_y;
    buffer->viewport_size_x = viewport_size_x;
    buffer->viewport_size_y = viewport_size_y;
}

internal void
tiled_text_buffer_set_offset(TiledTextBuffer *buffer, u32 offset_x, u32 offset_y)
{
    if (buffer == NULL)
        return;

    buffer->offset_x = offset_x;
    buffer->offset_y = offset_y;
}

internal TiledTextCell *
tiled_text_buffer_get_cell(TiledTextBuffer *buffer, u32 cell_index_x, u32 cell_index_y)
{
    if (buffer == NULL)
        return NULL;
    if (cell_index_x > buffer->cell_count_x || cell_index_y > buffer->cell_count_y)
        return NULL;

    const u32 cell_index = cell_index_x + (cell_index_y * buffer->cell_count_x);
    return buffer->cells + cell_index;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE(traian): Primitive drawing.
////////////////////////////////////////////////////////////////////////////////////////////////////

internal void
draw_clear_bitmap(Bitmap *bitmap, LinearColor clear_color)
{
    if (bitmap == NULL)
        return;

    if (bitmap->bytes_per_pixel == 4) {
        const u32 packed_color = linear_color_pack_to_u32(clear_color);
        u32 *dst_row = (u32 *)bitmap->pixels;
        
        for (u32 y = 0; y < bitmap->size_y; ++y) {
            for (u32 x = 0; x < bitmap->size_x; ++x) {
                dst_row[x] = packed_color;
            }
            dst_row += bitmap->size_x;
        }
    }
    else {
        // TODO(traian): Implement!
        ASSERT_NOT_REACHED;
    }
}

internal void
draw_quad(Bitmap *bitmap, s32 offset_x, s32 offset_y, u32 size_x, u32 size_y, LinearColor color)
{
    if (bitmap == NULL)
        return;

    offset_x = clamp_s32(offset_x, 0, bitmap->size_x);
    offset_y = clamp_s32(offset_y, 0, bitmap->size_y);
    const u32 max_x = clamp_s32(offset_x + size_x, 0, bitmap->size_x);
    const u32 max_y = clamp_s32(offset_y + size_y, 0, bitmap->size_y);
    size_x = max_x - offset_x;
    size_y = max_y - offset_y;

    if (bitmap->bytes_per_pixel == 4) {
        const u32 packed_color = linear_color_pack_to_u32(color);
        u32 *dst_row = (u32 *)bitmap_get_pixel_address(bitmap, offset_x, offset_y);

        for (u32 y = 0; y < size_y; ++y) {
            for (u32 x = 0; x < size_x; ++x) {
                dst_row[x] = packed_color;
            }
            dst_row += bitmap->size_x;
        }
    }
    else {
        // TODO(traian): Implement!
        ASSERT_NOT_REACHED;
    }
}

internal void
draw_rectangle(Bitmap *bitmap, s32 offset_x, s32 offset_y, u32 size_x, u32 size_y, u32 thickness, LinearColor color)
{
    const u32 thickness_x = thickness;
    const u32 thickness_y = thickness;

    // NOTE(traian): Bottom.
    draw_quad(bitmap,
        offset_x, offset_y,
        size_x - thickness_x, thickness_y,
        color);

    // NOTE(traian): Right.
    draw_quad(bitmap,
        offset_x + size_x - thickness_x, offset_y,
        thickness_x, size_y - thickness_y,
        color);

    // NOTE(traian): Top.
    draw_quad(bitmap,
        offset_x + thickness_x, offset_y + size_y - thickness_y,
        size_x - thickness_x, thickness_y,
        color);

    // NOTE(traian): Left.
    draw_quad(bitmap,
        offset_x, offset_y + thickness_y,
        thickness_x, size_y - thickness_y,
        color);
}

internal void
draw_rectangle_containing(Bitmap *bitmap, s32 offset_x, s32 offset_y, u32 size_x, u32 size_y, u32 thickness, LinearColor color)
{
    draw_rectangle(bitmap,
                   offset_x - thickness, offset_y - thickness,
                   size_x + 2 * thickness, size_y + 2 * thickness,
                   thickness, color);
}

internal void
draw_glyph_bitmap(Bitmap *bitmap, Bitmap *glyph_bitmap, s32 offset_x, s32 offset_y, LinearColor color,
                  u32 viewport_offset_x, u32 viewport_offset_y, u32 viewport_size_x, u32 viewport_size_y)
{
    if (bitmap == NULL || glyph_bitmap == NULL)
        return;
    ASSERT(glyph_bitmap->bytes_per_pixel == 1);

    // NOTE(traian): An invalid viewport rectangle was provided.
    if (viewport_offset_x + viewport_size_x > bitmap->size_x || viewport_offset_y + viewport_size_y > bitmap->size_y)
        return;

    Rect2s viewport_rect = rect2s(viewport_offset_x, viewport_offset_y, 
                                  viewport_offset_x + viewport_size_x,
                                  viewport_offset_y + viewport_size_y);
    Rect2s glyph_rect = rect2s(offset_x, offset_y,
                               offset_x + glyph_bitmap->size_x, offset_y + glyph_bitmap->size_y);
    Rect2s intersection_rect = rect2_intersect(viewport_rect, glyph_rect);
    if (rect2_is_degenerated(intersection_rect))
        return;

    const u32 glyph_offset_x = intersection_rect.min_x - glyph_rect.min_x;
    const u32 glyph_offset_y = intersection_rect.min_y - glyph_rect.min_y;
    u32 glyph_size_x, glyph_size_y;
    rect2_size(intersection_rect, &glyph_size_x, &glyph_size_y);

    if (bitmap->bytes_per_pixel == 4) {
        u32 *dst_row = (u32 *)bitmap_get_pixel_address(bitmap, intersection_rect.min_x, intersection_rect.min_y);
        u8 *src_row = (u8 *)bitmap_get_pixel_address(glyph_bitmap, glyph_offset_x, glyph_offset_y);

        for (u32 y = 0; y < glyph_size_y; ++y) {
            for (u32 x = 0; x < glyph_size_x; ++x) {
                const LinearColor current_color = linear_color_unpack_from_u32(dst_row[x]);
                const f32 alpha = (f32)src_row[x] / 255.0F;
                const LinearColor blended_color = linear_color_blend(current_color, color, alpha);
                dst_row[x] = linear_color_pack_to_u32(blended_color);
            }

            dst_row += bitmap->size_x;
            src_row += glyph_bitmap->size_x;
        }
    }
    else {
        // TODO(traian): Implement!
        ASSERT_NOT_REACHED;
    }
}

internal void
draw_tiled_text_buffer(Bitmap *bitmap, TiledTextBuffer *buffer, Font *font)
{
    if (bitmap == NULL || buffer == NULL)
        return;

    // NOTE(traian): Ensure no trivial invalid state.
    if (buffer->cell_size_x == 0 || buffer->cell_size_y == 0 || buffer->cell_count_x == 0 || buffer->cell_count_y == 0)
        return;
    if (buffer->viewport_size_x == 0 || buffer->viewport_size_y == 0)
        return;

    const u32 size_with_offset_x = buffer->viewport_size_x + buffer->offset_x;
    const u32 size_with_offset_y = buffer->viewport_size_y + buffer->offset_y;
    const u32 required_to_fill_x = required_to_fill_u32(buffer->cell_size_x, size_with_offset_x);
    const u32 required_to_fill_y = required_to_fill_u32(buffer->cell_size_y + buffer->line_spacing, size_with_offset_y + buffer->line_spacing);

    s32 base_cell_offset_x = buffer->viewport_offset_x - buffer->offset_x;
    s32 base_cell_offset_y = buffer->viewport_offset_y + buffer->viewport_size_y + buffer->offset_y - buffer->cell_size_y;

    for (u32 cell_index_y = 0; cell_index_y < required_to_fill_y; ++cell_index_y) {
        s32 cell_offset_x = base_cell_offset_x;
        s32 cell_offset_y = base_cell_offset_y - (cell_index_y * (buffer->cell_size_y + buffer->line_spacing));
        
        for (u32 cell_index_x = 0; cell_index_x < required_to_fill_x; ++cell_index_x) {
            TiledTextCell *cell = tiled_text_buffer_get_cell(buffer, cell_index_x, cell_index_y);
            if (ASCII_CHARACTER_FIRST <= cell->codepoint && cell->codepoint <= ASCII_CHARACTER_LAST) {
                FontGlyph *glyph = font_get_glyph(font, cell->codepoint);
                draw_glyph_bitmap(bitmap, &glyph->bitmap,
                    cell_offset_x + glyph->offset_x, cell_offset_y + glyph->offset_y, cell->color,
                    buffer->viewport_offset_x, buffer->viewport_offset_y,
                    buffer->viewport_size_x, buffer->viewport_size_y);
            }

            cell_offset_x += buffer->cell_size_x;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE(traian): Editor application.
////////////////////////////////////////////////////////////////////////////////////////////////////

struct EditorMemory
{
    MemoryArena *permanent_arena;
    MemoryArena *work_arena;
    MemoryArena *dynamic_resources_arena;
};

enum FontIDEnum : u8
{
    FONT_ID_DEFAULT = 0,
    FONT_ID_MAX_COUNT,
};
typedef u16 FontID;

struct EditorState
{
    EditorMemory   *memory;
    Bitmap         *offscreen_bitmap;
    Font            fonts[FONT_ID_MAX_COUNT];
    TiledTextBuffer tiled_text_buffer;
};

internal void
editor_on_resize(EditorState *state, u32 new_size_x, u32 new_size_y)
{
    Font *font = state->fonts + FONT_ID_DEFAULT;

    const u32 viewport_offset_x = 5;
    const u32 viewport_offset_y = 5;
    const u32 viewport_size_x = new_size_x / 2 - 10;
    const u32 viewport_size_y = new_size_y - 10;
    const u32 cell_size_x = font->advance;
    const u32 cell_size_y = font->ascent + font->descent;
    const u32 line_spacing = font->line_gap;
    const u32 offset_x = 0;
    const u32 offset_y = font->descent;

    u32 cell_count_x, cell_count_y;
    tiled_text_buffer_cell_count_from_viewport(viewport_size_x, viewport_size_y, cell_size_x, cell_size_y, line_spacing, true,
                                               &cell_count_x, &cell_count_y);
    tiled_text_buffer_initialize(&state->tiled_text_buffer, state->memory->dynamic_resources_arena,
                                 cell_count_x, cell_count_y);
    tiled_text_buffer_set_viewport(&state->tiled_text_buffer, viewport_offset_x, viewport_offset_y, viewport_size_x, viewport_size_y);
    tiled_text_buffer_set_cell_size(&state->tiled_text_buffer, cell_size_x, cell_size_y, line_spacing);
    tiled_text_buffer_set_offset(&state->tiled_text_buffer, offset_x, offset_y);

    FileReadResult result = platform_read_entire_file_to_arena("../../source/ocean_windows.cpp", state->memory->work_arena);
    const char *message = (const char *)result.file_data;

    u32 cell_index_x = 0;
    u32 cell_index_y = 0;
    while (*message) {
        const u32 codepoint = *message++;
        if (codepoint == '\r')
            continue;

        if (cell_index_x < cell_count_x && cell_index_y < cell_count_y) {
            TiledTextCell *cell = tiled_text_buffer_get_cell(&state->tiled_text_buffer, cell_index_x, cell_index_y);
            cell->codepoint = codepoint;
            cell->color = linear_color(200, 50, 70);
            ++cell_index_x;
        }

        if (codepoint == '\n') {
            cell_index_x = 0;
            ++cell_index_y;
        }
    }

    draw_clear_bitmap(state->offscreen_bitmap, linear_color(10, 10, 10));
    const u32 thickness = 5;
    draw_rectangle_containing(state->offscreen_bitmap, viewport_offset_x, viewport_offset_y,
                              viewport_size_x, viewport_size_y, thickness, linear_color(60, 100, 200));

    draw_tiled_text_buffer(state->offscreen_bitmap, &state->tiled_text_buffer, state->fonts + FONT_ID_DEFAULT);
}

internal void
editor_initialize_fonts(EditorState *state)
{
    FileReadResult result = platform_read_entire_file_to_arena("C:/Windows/Fonts/consola.ttf", state->memory->work_arena);
    font_initialize(state->fonts + FONT_ID_DEFAULT, state->memory->permanent_arena,
                    state->memory->work_arena, result.file_data, result.file_size,
                    30.0F);
}

internal EditorState *
editor_initialize(EditorMemory *memory, Bitmap *offscreen_bitmap)
{
    EditorState *state = ARENA_PUSH_STRUCT(memory->permanent_arena, EditorState);
    state->memory = memory;
    state->offscreen_bitmap = offscreen_bitmap;

    // NOTE(traian): Load all fonts required by the editor.
    editor_initialize_fonts(state);

    editor_on_resize(state, offscreen_bitmap->size_x, offscreen_bitmap->size_y);
    return state;
}

internal void
editor_update(EditorState *state)
{
}

internal void
editor_shutdown(EditorState *state)
{
}

#ifndef _WIN32
    #error Trying to compile the Windows platform layer!
#endif // _WIN32

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE(traian): Windows platform layer implementation.
////////////////////////////////////////////////////////////////////////////////////////////////////

PLATFORM_API_SIG_ALLOCATE_MEMORY(platform_allocate_memory)
{
    // NOTE(traian): Ensure consistent behaviour across all platforms. Requesting a zero-sized
    // memory block should yield an invalid pointer (NULL).
    if (allocation_size == 0)
        return NULL;

    DWORD allocation_type = 0;
    if (flags & ALLOCATE_MEMORY_FLAG_RESERVE) allocation_type |= MEM_RESERVE;
    if (flags & ALLOCATE_MEMORY_FLAG_COMMIT)  allocation_type |= MEM_COMMIT;

    void *allocation_block = VirtualAlloc(base_address, allocation_size, allocation_type, PAGE_READWRITE);
    return allocation_block;
}

PLATFORM_API_SIG_RELEASE_MEMORY(platform_release_memory)
{
    // NOTE(traian): Ensure consistent behaviour across all platforms. Requesting a zero-sized
    // memory block should yield an invalid pointer (NULL), and thus releasing an invalid pointer
    // should have no effects.
    if (base_address == NULL)
        return;

    DWORD free_type = 0;
    SIZE_T free_size = 0;

    if (flags & RELEASE_MEMORY_FLAG_DECOMMIT) { free_type |= MEM_DECOMMIT; free_size = allocation_size; }
    if (flags & RELEASE_MEMORY_FLAG_RELEASE)  { free_type |= MEM_RELEASE;  free_size = 0; }

    VirtualFree(base_address, allocation_size, free_type);
}

PLATFORM_API_SIG_READ_ENTIRE_FILE_TO_ARENA(platform_read_entire_file_to_arena)
{
    FileReadResult result = {};
    result.is_valid = false;
    result.file_data = NULL;
    result.file_size = 0;

    HANDLE file_handle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL,
                                     OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file_handle == INVALID_HANDLE_VALUE)
        return result;

    LARGE_INTEGER file_size = {};
    if (!GetFileSizeEx(file_handle, &file_size)) {
        CloseHandle(file_handle);
        return result;
    }

    // NOTE(traian): Allocate the memory required to store the file in memory from the provided arena.
    void *file_data = memory_arena_allocate(arena, file_size.QuadPart);
    ASSERT(file_size.QuadPart <= 0xFFFFFFFF);

    DWORD number_of_bytes_read = 0;
    bool read_success = ReadFile(file_handle, file_data, (DWORD)file_size.QuadPart, &number_of_bytes_read, NULL);
    if (!read_success || number_of_bytes_read != file_size.QuadPart) {
        CloseHandle(file_handle);
        // NOTE(traian): The memory allocated from the arena is esentially "leaked"!
        return result;
    }

    result.is_valid = true;
    result.file_data = file_data;
    result.file_size = file_size.QuadPart;

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE(traian): Windows platform layer start-up.
////////////////////////////////////////////////////////////////////////////////////////////////////

internal void
win32_get_window_size(HWND window_handle, u32 *out_size_x, u32 *out_size_y)
{
    RECT window_rect = {};
    GetClientRect(window_handle, &window_rect);
    *out_size_x = window_rect.right - window_rect.left;
    *out_size_y = window_rect.bottom - window_rect.top;
}

struct Win32OffscreenBitmap
{
    Bitmap     bitmap;
    BITMAPINFO info;
    HDC        device_context;
};

internal void
win32_sync_offscreen_bitmap_with_window(Win32OffscreenBitmap* bitmap, MemoryArena *arena, HWND window_handle)
{
    u32 window_size_x, window_size_y;
    win32_get_window_size(window_handle, &window_size_x, &window_size_y);
    if (bitmap->bitmap.size_x == window_size_x && bitmap->bitmap.size_y == window_size_y)
        return;

    bitmap->info.bmiHeader.biSize = sizeof(bitmap->info.bmiHeader);
    bitmap->info.bmiHeader.biPlanes = 1;
    bitmap->info.bmiHeader.biBitCount = 8 * 4;
    bitmap->info.bmiHeader.biCompression = BI_RGB;
    bitmap->info.bmiHeader.biWidth = window_size_x;
    bitmap->info.bmiHeader.biHeight = window_size_y;

    if (bitmap->device_context == NULL)
        bitmap->device_context = GetDC(window_handle);

    bitmap_initialize(&bitmap->bitmap, arena, window_size_x, window_size_y, 4);
}

internal void
win32_present_offscreen_bitmap(Win32OffscreenBitmap* bitmap, HWND window_handle)
{
    u32 window_size_x, window_size_y;
    win32_get_window_size(window_handle, &window_size_x, &window_size_y);

    StretchDIBits(bitmap->device_context,
        0, 0, window_size_x, window_size_y,
        0, 0, bitmap->bitmap.size_x, bitmap->bitmap.size_y,
        bitmap->bitmap.pixels, &bitmap->info, DIB_RGB_COLORS, SRCCOPY);
}

global_variable bool g_window_should_close = false;

internal LRESULT
win32_window_procedure(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param)
{
    switch (message) {
        case WM_CLOSE:
        case WM_QUIT: {
            g_window_should_close = true;
            return 0;
        }
    }

    return DefWindowProcA(window_handle, message, w_param, l_param);
}

INT
WinMain(HINSTANCE instance, HINSTANCE previous_instance, LPSTR command_line, INT show_command)
{
    WNDCLASSA window_class = {};
    window_class.lpszClassName = "OceanWindowClass";
    window_class.hInstance = instance;
    window_class.lpfnWndProc = win32_window_procedure;
    RegisterClassA(&window_class);

    DWORD window_style_flags = WS_OVERLAPPEDWINDOW;
    HWND window_handle = CreateWindowA(
        "OceanWindowClass", "ocean @ AVR | Windows 64-bit Development",
        window_style_flags, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, instance, NULL);
    if (window_handle == NULL) {
        // NOTE(traian): Without a window there is nothing we can do.
        return 1;
    }
    ShowWindow(window_handle, SW_MAXIMIZE);

    // NOTE(traian): Designed to allocate resources that are immutable in size and will be used for
    // the entire duration of the application life.
    MemoryArena permanent_arena = {};
    memory_arena_initialize(&permanent_arena, MEGABYTES(2), GIGABYTES(1));

    // NOTE(traian): Designed to provide very short-lived memory, useful for intermediate operations.
    // This arena is reseted before each editor update cycle, so no memory that persists more than
    // the current editor update cycle should be allocated from this arena!
    MemoryArena work_arena = {};
    memory_arena_initialize(&work_arena, MEGABYTES(16), GIGABYTES(4));

    // NOTE(traian): Designed to allocate resources that are directly linked with the editor window size.
    // Due to this link, this arena is reseted after each window resize event, and thus all resources
    // allocated from it must be reinitialized after each resize.
    MemoryArena dynamic_resources_arena = {};
    memory_arena_initialize(&dynamic_resources_arena, MEGABYTES(32), GIGABYTES(8));
    
    EditorMemory editor_memory = {};
    editor_memory.permanent_arena = &permanent_arena;
    editor_memory.work_arena = &work_arena;
    editor_memory.dynamic_resources_arena = &dynamic_resources_arena;

    Win32OffscreenBitmap offscreen_bitmap = {};
    win32_sync_offscreen_bitmap_with_window(&offscreen_bitmap, editor_memory.dynamic_resources_arena, window_handle);
    EditorState *editor_state = editor_initialize(&editor_memory, &offscreen_bitmap.bitmap);

    g_window_should_close = false;
    MSG message = {};
    while (!g_window_should_close && GetMessageA(&message, window_handle, 0, 0)) {
        TranslateMessage(&message);
        DispatchMessageA(&message);

        // NOTE(traian): The work arena is designed only to allocate memory for immediate operations.
        // No memory allocated from this arena should persist for more than the duration of the last editor update,
        // and thus is it safe to reset it everytime.
        memory_arena_reset(editor_memory.work_arena);

        u32 window_size_x, window_size_y;
        win32_get_window_size(window_handle, &window_size_x, &window_size_y);
        if (window_size_x > 0 && window_size_y > 0) {
            if (offscreen_bitmap.bitmap.size_x != window_size_x || offscreen_bitmap.bitmap.size_y != window_size_y) {
                memory_arena_reset(editor_memory.dynamic_resources_arena);
                win32_sync_offscreen_bitmap_with_window(&offscreen_bitmap, editor_memory.dynamic_resources_arena, window_handle);
                editor_on_resize(editor_state, window_size_x, window_size_y);
            }

            editor_update(editor_state);
            win32_present_offscreen_bitmap(&offscreen_bitmap, window_handle);
        }
    }

    editor_shutdown(editor_state);
    CloseWindow(window_handle);
    return 0;
}
