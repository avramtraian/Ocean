/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#include <Core/Assertion.h>
#include <Platform/Memory.h>
#include <Platform/OffscreenBitmap.h>
#include <Platform/Windows/WindowsHeaders.h>

typedef struct WindowsOffscrenBitmapData {
    BITMAPINFO info;
    HDC device_context;
} WindowsOffscrenBitmapData;

void platform_offscreen_bitmap_create(OffscreenBitmap* offscreen_bitmap, Window window)
{
    VERIFY(!offscreen_bitmap->bitmap.pixels);

    const u32 window_width = platform_window_width(window);
    const u32 window_height = platform_window_height(window);

    constexpr usize bytes_per_pixel = 4;
    constexpr BitmapFormat bitmap_format = BITMAP_FORMAT_B8G8R8A8;

    const usize bitmap_byte_count = (usize)window_width * (usize)window_height * bytes_per_pixel + sizeof(WindowsOffscrenBitmapData);

    ReadWriteBytes bitmap_data = platform_memory_allocate(bitmap_byte_count);
    WindowsOffscrenBitmapData* internal_data = (WindowsOffscrenBitmapData*)bitmap_data;
    ReadWriteBytes pixels = bitmap_data + sizeof(WindowsOffscrenBitmapData);

    internal_data->info.bmiHeader.biSize = sizeof(internal_data->info.bmiHeader);
    internal_data->info.bmiHeader.biWidth = window_width;
    internal_data->info.bmiHeader.biHeight = window_height;
    internal_data->info.bmiHeader.biPlanes = 1;
    internal_data->info.bmiHeader.biBitCount = bytes_per_pixel * 8;
    internal_data->info.bmiHeader.biCompression = BI_RGB;

    internal_data->device_context = GetDC((HWND)platform_window_native_handle(window));

    bitmap_create_from_memory(&offscreen_bitmap->bitmap, window_width, window_height, bitmap_format, pixels);
    offscreen_bitmap->window = window;
    offscreen_bitmap->internal_data = internal_data;
}

void platform_offscreen_bitmap_destroy(OffscreenBitmap* offscreen_bitmap)
{
    WindowsOffscrenBitmapData* internal_data = (WindowsOffscrenBitmapData*)offscreen_bitmap->internal_data;
    ReleaseDC((HWND)platform_window_native_handle(offscreen_bitmap->window), internal_data->device_context);

    constexpr usize bytes_per_pixel = 4;
    const usize bitmap_byte_count =
        (usize)offscreen_bitmap->bitmap.width * (usize)offscreen_bitmap->bitmap.height * bytes_per_pixel + sizeof(WindowsOffscrenBitmapData);

    // NOTE: The bitmap memory buffer stores both the internal data and the pixels, in this order.
    //       As far as the platform allocator is concerned, the memory block is represented by the bitmap internal data pointer.
    platform_memory_release(offscreen_bitmap->internal_data, bitmap_byte_count);

    offscreen_bitmap->bitmap.width = 0;
    offscreen_bitmap->bitmap.height = 0;
    offscreen_bitmap->bitmap.format = BITMAP_FORMAT_UNKNOWN;
    offscreen_bitmap->bitmap.pixels = NULL;

    offscreen_bitmap->window = NULL;
    offscreen_bitmap->internal_data = NULL;
}

void platform_offscreen_bitmap_draw(const OffscreenBitmap* offscreen_bitmap)
{
    WindowsOffscrenBitmapData* internal_data = (WindowsOffscrenBitmapData*)offscreen_bitmap->internal_data;

    StretchDIBits(
        internal_data->device_context,
        0,
        0,
        platform_window_width(offscreen_bitmap->window),
        platform_window_height(offscreen_bitmap->window),
        0,
        0,
        offscreen_bitmap->bitmap.width,
        offscreen_bitmap->bitmap.height,
        offscreen_bitmap->bitmap.pixels,
        &internal_data->info,
        DIB_RGB_COLORS,
        SRCCOPY
    );
}

void platform_offscreen_bitmap_resize_synced(OffscreenBitmap* offscreen_bitmap)
{
    const u32 window_current_width = platform_window_width(offscreen_bitmap->window);
    const u32 window_current_height = platform_window_height(offscreen_bitmap->window);

    if (offscreen_bitmap->bitmap.width != window_current_width || offscreen_bitmap->bitmap.height != window_current_height) {
        Window window = offscreen_bitmap->window;
        platform_offscreen_bitmap_destroy(offscreen_bitmap);
        platform_offscreen_bitmap_create(offscreen_bitmap, window);
    }
}
