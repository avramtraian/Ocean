/*
 * Copyright (c) 2025-2026 Traian Avram. All rights reserved.
 * This file is part of my personal text editor and is distributed under the MIT license.
 */

struct Vector2u {
    u32 x;
    u32 y;
};

struct Vector2s {
    s32 x;
    s32 y;
};

struct Rect2D {
    Vector2s min;
    Vector2s max;
};

struct LinearColor {
    alignas(u32) u8 b;
    u8 g;
    u8 r;
    u8 a;
};

template<typename T> inline T
min(T a, T b)
{
    T result = (a < b) ? a : b;
    return result;
}

template<typename T> inline T
max(T a, T b)
{
    T result = (a > b) ? a : b;
    return result;
}

template<typename T> inline T
clamp(T value, T min_bound, T max_bound)
{
    T result = min(max_bound, max(min_bound, value));
    return result;
}

template<typename T> inline T
lerp(T a, T b, f32 t)
{
    T result = a + ((f32)b - (f32)a) * t;
    return result;
}

template<typename T> inline T
align_to_pow2(T value, T alignment)
{
    T alignment_mask = alignment - 1;
    T result = (value + alignment_mask) & (~alignment_mask);
    return result;
}

inline Vector2s
v2s(s32 x, s32 y)
{
    Vector2s result;
    result.x = x;
    result.y = y;
    return result;
}

inline Vector2s
to_v2s(Vector2u vector2u)
{
    Vector2s result;
    result.x = (s32)vector2u.x; // @Incomplete: Check for overflow!
    result.y = (s32)vector2u.y; // @Incomplete: Check for overflow!
    return result;
}

inline Vector2u
to_v2u(Vector2s vector2s)
{
    Vector2u result;
    result.x = (u32)vector2s.x; // @Incomplete: Check for overflow!
    result.y = (u32)vector2s.y; // @Incomplete: Check for overflow!
    return result;
}

inline Rect2D
rect(s32 min_x, s32 min_y, s32 max_x, s32 max_y)
{
    Rect2D result;
    result.min = v2s(min_x, min_y);
    result.max = v2s(max_x, max_y);
    return result;
}

inline Rect2D
rect_offset_size(s32 offset_x, s32 offset_y, u32 size_x, u32 size_y)
{
    Rect2D result;
    result.min.x = offset_x;
    result.min.y = offset_y;
    result.max.x = offset_x + size_x;
    result.max.y = offset_y + size_y;
    return result;
}

inline Rect2D
rect_offset_size(Vector2s offset, Vector2u size)
{
    Rect2D result;
    result.min.x = offset.x;
    result.min.y = offset.y;
    result.max.x = offset.x + size.x;
    result.max.y = offset.y + size.y;
    return result;
}

inline u32
rect_size_x(Rect2D rectangle)
{
    u32 result = rectangle.max.x - rectangle.min.x;
    return result;
}

inline u32
rect_size_y(Rect2D rectangle)
{
    u32 result = rectangle.max.y - rectangle.min.y;
    return result;
}

inline Rect2D
rect_intersect(Rect2D a, Rect2D b)
{
    Rect2D result;
    result.min.x = clamp(a.min.x, b.min.x, b.max.x);
    result.min.y = clamp(a.min.y, b.min.y, b.max.y);
    result.max.x = clamp(a.max.x, b.min.x, b.max.x);
    result.max.y = clamp(a.max.y, b.min.y, b.max.y);
    return result;
}

inline bool
is_degenerated(Rect2D rectangle)
{
    const bool result = (rectangle.min.x >= rectangle.max.x) ||
                        (rectangle.min.y >= rectangle.max.y);
    return result;
}

inline LinearColor
linear_color(u8 r, u8 g, u8 b, u8 a = 0xFF)
{
    LinearColor result;
    result.b = b;
    result.g = g;
    result.r = r;
    result.a = a;
    return result;
}

inline LinearColor
lerp(LinearColor a, LinearColor b, f32 t)
{
    LinearColor result;
    result.b = lerp(a.b, b.b, t);
    result.g = lerp(a.g, b.g, t);
    result.r = lerp(a.r, b.r, t);
    result.a = lerp(a.a, b.a, t);
    return result;
}

inline u32
pack_bgra(LinearColor color)
{
    u32 result = *(u32*)&color;
    return result;
}

inline LinearColor
unpack_bgra_to_linear_color(u32 packed_color)
{
    LinearColor result = *(LinearColor*)&packed_color;
    return result;
}