/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#include <Draw/Bitmap.h>
#include <Platform/Memory.h>
#include <Platform/OffscreenBitmap.h>
#include <Platform/Windows/WindowsHeaders.h>

typedef struct WindowsSoftwareOffscreenBitmap {
    Bitmap bitmap;
    Window window;
    BITMAPINFO info;
    HDC device_context;
} WindowsSoftwareOffscreenBitmap;

OffscreenBitmap offscreen_bitmap_create(LinearArena* arena, Window window)
{
    WindowsSoftwareOffscreenBitmap* offscreen_bitmap =
        (WindowsSoftwareOffscreenBitmap*)core_linear_arena_allocate(arena, sizeof(WindowsSoftwareOffscreenBitmap));

    const u32 window_width = platform_window_width(window);
    const u32 window_height = platform_window_height(window);
    constexpr usize bytes_per_pixel = 4;

    offscreen_bitmap->info.bmiHeader.biSize = sizeof(offscreen_bitmap->info.bmiHeader);
    offscreen_bitmap->info.bmiHeader.biWidth = window_width;
    offscreen_bitmap->info.bmiHeader.biHeight = window_height;
    offscreen_bitmap->info.bmiHeader.biPlanes = 1;
    offscreen_bitmap->info.bmiHeader.biBitCount = bytes_per_pixel * 8;
    offscreen_bitmap->info.bmiHeader.biCompression = BI_RGB;

    offscreen_bitmap->device_context = GetDC((HWND)platform_window_native_handle(window));

    const usize bitmap_byte_count = (usize)window_width * (usize)window_height * bytes_per_pixel;
    ReadWriteBytes bitmap_data = platform_memory_allocate(bitmap_byte_count);
    offscreen_bitmap->bitmap =
        bitmap_create_from_memory(arena, window_width, window_height, BITMAP_FORMAT_B8G8R8A8, read_write_byte_span(bitmap_data, bitmap_byte_count));

    offscreen_bitmap->window = window;
    return offscreen_bitmap;
}

void offscreen_bitmap_destroy(OffscreenBitmap* offscreen_bitmap_handle)
{
    WindowsSoftwareOffscreenBitmap* offscreen_bitmap = (WindowsSoftwareOffscreenBitmap*)(*offscreen_bitmap_handle);

    ReleaseDC((HWND)platform_window_native_handle(offscreen_bitmap->window), offscreen_bitmap->device_context);
    platform_memory_release(bitmap_get_data(offscreen_bitmap->bitmap), bitmap_get_byte_count(offscreen_bitmap->bitmap));
    bitmap_destroy(&offscreen_bitmap->bitmap);

    offscreen_bitmap->device_context = NULL;
    offscreen_bitmap->window = INVALID_HANDLE;

    *offscreen_bitmap_handle = INVALID_HANDLE;
}

void offscreen_bitmap_swap(OffscreenBitmap offscreen_bitmap_handle)
{
    WindowsSoftwareOffscreenBitmap* offscreen_bitmap = (WindowsSoftwareOffscreenBitmap*)offscreen_bitmap_handle;

    StretchDIBits(
        offscreen_bitmap->device_context,
        0,
        0,
        platform_window_width(offscreen_bitmap->window),
        platform_window_height(offscreen_bitmap->window),
        0,
        0,
        bitmap_get_width(offscreen_bitmap->bitmap),
        bitmap_get_height(offscreen_bitmap->bitmap),
        bitmap_get_data(offscreen_bitmap->bitmap),
        &offscreen_bitmap->info,
        DIB_RGB_COLORS,
        SRCCOPY
    );
}

void offscreen_bitmap_resize_synced(OffscreenBitmap offscreen_bitmap_handle)
{
    WindowsSoftwareOffscreenBitmap* offscreen_bitmap = (WindowsSoftwareOffscreenBitmap*)offscreen_bitmap_handle;
    const u32 window_width = platform_window_width(offscreen_bitmap->window);
    const u32 window_height = platform_window_height(offscreen_bitmap->window);

    if (bitmap_get_width(offscreen_bitmap->bitmap) == window_width && bitmap_get_height(offscreen_bitmap->bitmap) == window_height)
        return;

    // Reallocate the bitmap data.
    platform_memory_release(bitmap_get_data(offscreen_bitmap->bitmap), bitmap_get_byte_count(offscreen_bitmap->bitmap));
    const usize new_bitmap_byte_count = (usize)window_width * (usize)window_height * bitmap_get_bytes_per_pixel(offscreen_bitmap->bitmap);
    ReadWriteBytes new_bitmap_data = platform_memory_allocate(new_bitmap_byte_count);

    bitmap_resize_from_memory(offscreen_bitmap->bitmap, window_width, window_height, read_write_byte_span(new_bitmap_data, new_bitmap_byte_count));

    offscreen_bitmap->info.bmiHeader.biWidth = window_width;
    offscreen_bitmap->info.bmiHeader.biHeight = window_height;
}

Bitmap offscreen_bitmap_get_bitmap(OffscreenBitmap offscreen_bitmap_handle)
{
    WindowsSoftwareOffscreenBitmap* offscreen_bitmap = (WindowsSoftwareOffscreenBitmap*)offscreen_bitmap_handle;
    return offscreen_bitmap->bitmap;
}
