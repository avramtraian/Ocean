/**
 * Copyright (c) 2025 Traian Avram. All rights reserved.
 * This source file is part of the Ocean text editor.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#include "core.h"
#include "draw.h"
#include "editor.h"
#include "graphics.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE(traian): Windows platform layer implementation.
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _WIN32
    #error Trying to compile the Windows platform layer!
#endif // _WIN32

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

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
                editor_resize(editor_state, window_size_x, window_size_y);
            }

            editor_update(editor_state);
            win32_present_offscreen_bitmap(&offscreen_bitmap, window_handle);
        }
    }

    editor_destroy(editor_state);
    CloseWindow(window_handle);
    return 0;
}
