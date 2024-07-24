/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#include <Platform/SoftwareSwapchain.h>
#include <Platform/Windows/WindowsHeaders.h>

void platform_software_swapchain_swap(Window window, u32 graphics_bitmap_width, u32 graphics_bitmap_height, void* graphics_bitmap_data)
{
    BITMAPINFO info = {};
    info.bmiHeader.biSize = sizeof(info.bmiHeader);
    info.bmiHeader.biWidth = graphics_bitmap_width;
    info.bmiHeader.biHeight = graphics_bitmap_height;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;

    HDC device_context = GetDC((HWND)platform_window_native_handle(window));

    StretchDIBits(
        device_context,
        0,
        0,
        platform_window_width(window),
        platform_window_height(window),
        0,
        0,
        graphics_bitmap_width,
        graphics_bitmap_height,
        graphics_bitmap_data,
        &info,
        DIB_RGB_COLORS,
        SRCCOPY
    );

    ReleaseDC((HWND)platform_window_native_handle(window), device_context);
}
