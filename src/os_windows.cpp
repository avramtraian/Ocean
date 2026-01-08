/*
 * Copyright (c) 2025-2026 Traian Avram. All rights reserved.
 * This file is part of my personal text editor and is distributed under the MIT license.
 */

#pragma comment(lib, "user32")
#pragma comment(lib, "gdi32")
#pragma comment(lib, "freetype")

#define PLATFORM_OS_WINDOWS 1
#define PLATFORM_OS_LINUX   0
#define PLATFORM_OS_MACOS   0

#if _WIN64
    #define PLATFORM_ARCH_X86    0
    #define PLATFORM_ARCH_X86_64 1
    #define PLATFORM_ARCH_ARM32  0
    #define PLATFORM_ARCH_ARM64  0
#else
    #define PLATFORM_ARCH_X86    1
    #define PLATFORM_ARCH_X86_64 0
    #define PLATFORM_ARCH_ARM32  0
    #define PLATFORM_ARCH_ARM64  0
#endif // _WIN64

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <math.h>
#include <malloc.h>
#include <stdio.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/ftlcdfil.h>

#define internal         static
#define function         static
#define local_persistent static
#define constant         static constexpr

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;

typedef signed char      s8;
typedef signed short     s16;
typedef signed int       s32;
typedef signed long long s64;

typedef float  f32;
typedef double f64;

#if PLATFORM_ARCH_X86 || PLATFORM_ARCH_ARM32
typedef u32 usize;
typedef s32 ssize;
typedef u32 uintptr;
#endif // PLATFORM_ARCH_X86 || PLATFORM_ARCH_ARM32

#if PLATFORM_ARCH_X86_64 || PLATFORM_ARCH_ARM64
typedef u64 usize;
typedef s64 ssize;
typedef u64 uintptr;
#endif // PLATFORM_ARCH_X86_64 || PLATFORM_ARCH_ARM64

//
// DECLARATION OF THE PLATFORM LAYER:
//

struct OSReadFileResult {
    bool is_valid;
    u8* data;
    usize size;
};

struct OSWindowBitmap {
    constant usize bytes_per_pixel = 4;
    u8* pixels;
    u32 size_x;
    u32 size_y;
    usize number_of_rows;
    usize bytes_per_row;
    usize allocation_size;
};

internal void               win32_handle_assertion_failed   (char* expression, char* file_name, char* function_signature, int line_number);
internal struct Vector2u    win32_get_window_size           ();
internal LRESULT            win32_window_procedure          (HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param);

internal usize              os_get_memory_page_size         ();
internal bool               os_is_memory_page_aligned       (usize value);
internal usize              os_get_memory_page_aligned      (usize value);
internal void*              os_reserve_memory               (usize size);
internal void               os_commit_memory                (void* address, usize size);
internal void*              os_allocate_memory              (usize size);
internal void               os_free_memory                  (void* address);
internal void               os_decommit_memory              (void* address, usize size);
internal OSReadFileResult   os_read_entire_file             (char* file_name);
internal void               os_free_read_file_result        (OSReadFileResult result);

#define ASSERT(...)                                                                   \
    if (!(__VA_ARGS__)) {                                                             \
        win32_handle_assertion_failed(#__VA_ARGS__, __FILE__, __FUNCSIG__, __LINE__); \
        __debugbreak();                                                               \
    }

#define ASSERT_NOT_REACHED                                                    \
    {                                                                         \
        win32_handle_assertion_failed(NULL, __FILE__, __FUNCSIG__, __LINE__); \
        __debugbreak();                                                       \
        __assume(false);                                                      \
    }

internal HWND           g_window_handle;
internal bool           g_window_should_close;
internal OSWindowBitmap g_window_bitmap;

//
// IMPLEMENTATIONS OF ALL OTHER LAYERS:
//

#include "math.cpp"
#include "core.cpp"
#include "unicode.cpp"
#include "font.cpp"
#include "state.cpp"
#include "render.cpp"
#include "update.cpp"

//
// IMPLEMENTATION OF THE PLATFORM-AGNOSTIC OS INTERFACE:
//

internal usize
os_get_memory_page_size()
{
    local_persistent usize s_page_size = 0;
    if (s_page_size == 0) {
        SYSTEM_INFO system_info = {};
        GetSystemInfo(&system_info);
        s_page_size = system_info.dwPageSize;
    }

    usize result = s_page_size;
    return result;
}

internal bool
os_is_memory_page_aligned(usize value)
{
    usize page_size_mask = os_get_memory_page_size() - 1; // Not guaranteed, but on all modern platforms we can safely assume this is a power of 2.
    bool result = ((value & page_size_mask) == 0);
    return result;
}

internal usize
os_get_memory_page_aligned(usize value)
{
    usize page_size = os_get_memory_page_size();
    usize result = align_to_pow2(value, page_size); // Not guaranteed, but on all modern platforms we can safely assume this is a power of 2.
    return result;
}

internal void*
os_reserve_memory(usize size)
{
    if (size == 0)
        return NULL;

    ASSERT(os_is_memory_page_aligned(size));
    void* result = VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_READWRITE);
    return result;
}

internal void
os_commit_memory(void* address, usize size)
{
    ASSERT(os_is_memory_page_aligned(size));
    VirtualAlloc(address, size, MEM_COMMIT, PAGE_READWRITE);
}

internal void*
os_allocate_memory(usize size)
{
    if (size == 0)
        return NULL;

    ASSERT(os_is_memory_page_aligned(size));
    void* result = VirtualAlloc(NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    return result;
}

internal void
os_free_memory(void* address)
{
    if (address == NULL)
        return;
    VirtualFree(address, 0, MEM_RELEASE);
}

internal void
os_decommit_memory(void* address, usize size)
{
    ASSERT(os_is_memory_page_aligned(size));
    VirtualFree(address, size, MEM_DECOMMIT);
}

internal OSReadFileResult
os_read_entire_file(char* file_name)
{
    OSReadFileResult result = {};
    result.is_valid = false;

    HANDLE file_handle = CreateFileA(file_name, GENERIC_READ, FILE_SHARE_READ, NULL,
                                     OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file_handle == INVALID_HANDLE_VALUE)
        return result; // Failed to open the file handle.

    LARGE_INTEGER file_size = {};
    if (!GetFileSizeEx(file_handle, &file_size)) {
        CloseHandle(file_handle);
        return result; // Failed to query the file size.
    }

    if (file_size.QuadPart == 0) {
        result.is_valid = true;
        result.data = NULL;
        result.size = 0;
        return result;
    }

#if PLATFORM_ARCH_X86 || PLATFORM_ARCH_ARM32
    if (file_size.QuadPart > (usize)(-1)) {
        CloseHandle(file_handle);
        return result; // We can't possibly load the entire file into memory.
    }
#endif // PLATFORM_ARCH_X86 || PLATFORM_ARCH_ARM32

    u8* file_data = (u8*)os_allocate_memory(os_get_memory_page_aligned(file_size.QuadPart));
    if (file_data == NULL) {
        CloseHandle(file_handle);
        return result; // Failed to allocate enough memory to store the contents of the file.
    }

    u64 bytes_read_so_far = 0;
    constant u64 MAX_BYTES_TO_READ = (DWORD)(-1);

    while (bytes_read_so_far < file_size.QuadPart) {
        DWORD bytes_to_read = (DWORD)min(file_size.QuadPart - bytes_read_so_far, MAX_BYTES_TO_READ);
        DWORD bytes_actually_read = 0;

        if (!ReadFile(file_handle, file_data + bytes_read_so_far, bytes_to_read, &bytes_actually_read, NULL)) {
            CloseHandle(file_handle);
            os_free_memory(file_data);
            return result; // Failed to read from the file.
        }

        bytes_read_so_far += bytes_actually_read;
    }

    result.is_valid = true;
    result.data = file_data;
    result.size = file_size.QuadPart;
    return result;
}

internal void
os_free_read_file_result(OSReadFileResult result)
{
    if (result.is_valid && result.size > 0)
        os_free_memory(result.data);
}

//
// IMPLEMENTATION OF THE PURE WIN32 LAYER:
//

internal void
win32_handle_assertion_failed(char* expression, char* file_name, char* function_signature, int line_number)
{
    if (expression == NULL)
        expression = "Assertion not reached";

    char message[4096] = {};
    snprintf(message, sizeof(message), "%s\n%s\n%s:%d", expression, function_signature, file_name, line_number);

    OutputDebugStringA(message);
    MessageBoxA(NULL, message, "Assertion triggered!", MB_OK | MB_ICONERROR);
}

internal Vector2u
win32_get_window_size()
{
    RECT window_client_rect = {};
    GetClientRect(g_window_handle, &window_client_rect);
    
    Vector2u result;
    result.x = window_client_rect.right - window_client_rect.left;
    result.y = window_client_rect.bottom - window_client_rect.top;
    return result;
}

internal LRESULT
win32_window_procedure(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param)
{
    switch (message) {
      case WM_QUIT:
      case WM_CLOSE: {
        g_window_should_close = true;
        return 0;
      }
    }

    return DefWindowProcA(window_handle, message, w_param, l_param);
}

INT WINAPI
WinMain(HINSTANCE current_instance, HINSTANCE previous_instance, LPSTR command_line, INT show_command)
{
    // NOTE(Traian): Why on earth isn't this the default value?????
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    WNDCLASSA window_class = {};
    window_class.lpszClassName = "Editor3WindowClass";
    window_class.lpfnWndProc = win32_window_procedure;
    window_class.hInstance = current_instance;
    RegisterClassA(&window_class);

    g_window_handle = CreateWindowA("Editor3WindowClass", "editor3", WS_OVERLAPPEDWINDOW,
                                    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                    NULL, NULL, current_instance, NULL);
    ShowWindow(g_window_handle, show_command);
    HDC device_context = GetDC(g_window_handle);

    GlobalFontsDescription fonts_description = {};
    fonts_description.text_name     = "../DroidSansMono.ttf";
    fonts_description.ui_name       = "../DroidSansMono.ttf";
    fonts_description.ui_small_name = "../DroidSansMono.ttf";
    fonts_description.ui_big_name   = "../DroidSansMono.ttf";
    // fonts_description.text_name     = "C:/Windows/Fonts/consola.ttf";
    // fonts_description.ui_name       = "C:/Windows/Fonts/consola.ttf";
    // fonts_description.ui_small_name = "C:/Windows/Fonts/consola.ttf";
    // fonts_description.ui_big_name   = "C:/Windows/Fonts/consola.ttf";
    fonts_description.text_pixel_height     = 20;
    fonts_description.ui_pixel_height       = 30;
    fonts_description.ui_small_pixel_height = 20;
    fonts_description.ui_big_pixel_height   = 40;
    
    MemoryArena fonts_arena = create_arena(KiB(128), MiB(64));
    reload_global_fonts(&fonts_description, &fonts_arena);

    EditorState editor_state = {};
    initialize_editor(&editor_state);

    g_window_should_close = false;
    while (!g_window_should_close) {
        // Process the message queue.
        MSG window_message = {};
        while (PeekMessageA(&window_message, g_window_handle, 0, 0, PM_REMOVE)) {
            TranslateMessage(&window_message);
            DispatchMessageA(&window_message);
        }
        Vector2u window_size = win32_get_window_size();

        // Resize the window bitmap such that it matches the window client region.
        usize min_bytes_per_row = align_to_pow2(window_size.x, (u32)4) * g_window_bitmap.bytes_per_pixel;
        usize min_number_of_rows = align_to_pow2(window_size.y, (u32)4);
        usize min_allocation_size = min_number_of_rows * min_bytes_per_row;
        if (min_allocation_size > g_window_bitmap.allocation_size) {
            os_free_memory(g_window_bitmap.pixels);
            g_window_bitmap.allocation_size = os_get_memory_page_aligned(min_allocation_size);
            g_window_bitmap.pixels = (u8*)os_allocate_memory(g_window_bitmap.allocation_size);
        }

        g_window_bitmap.bytes_per_row = min_bytes_per_row;
        g_window_bitmap.number_of_rows = min_number_of_rows;
        g_window_bitmap.size_x = window_size.x;
        g_window_bitmap.size_y = window_size.y;
        zero_memory(g_window_bitmap.pixels, g_window_bitmap.number_of_rows * g_window_bitmap.bytes_per_row);

        update_editor(&editor_state);
        render_editor_frame(&editor_state);

        // Display the window bitmap to the screen.
        BITMAPINFO bitmap_info = {};
        bitmap_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bitmap_info.bmiHeader.biWidth = g_window_bitmap.bytes_per_row / g_window_bitmap.bytes_per_pixel;
        bitmap_info.bmiHeader.biHeight = g_window_bitmap.number_of_rows;
        bitmap_info.bmiHeader.biPlanes = 1;
        bitmap_info.bmiHeader.biBitCount = 8 * g_window_bitmap.bytes_per_pixel;
        bitmap_info.bmiHeader.biCompression = BI_RGB;
        StretchDIBits(device_context, 0, 0, window_size.x, window_size.y,
                      0, 0, g_window_bitmap.size_x, g_window_bitmap.size_y,
                      g_window_bitmap.pixels, &bitmap_info, DIB_RGB_COLORS, SRCCOPY);
    }

    return 0;
}
