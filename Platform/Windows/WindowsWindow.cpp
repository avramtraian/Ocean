/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#include <Core/Assertion.h>
#include <Core/Memory/Arena.h>
#include <Platform/Window.h>
#include <cstdio>

#if !OCEAN_PLATFORM_WINDOWS
#    error Trying to include the Windows headers but they are not available!
#endif // OCEAN_PLATFORM_WINDOWS

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

struct WindowsWindow {
    HWND handle { nullptr };
};

static LRESULT platform_windows_window_procedure(HWND, UINT, WPARAM, LPARAM);

Window platform_window_create(LinearArena& arena)
{
    constexpr const char* window_class_name = "OceanWindowClass";

    // Register the window class if it is the first time calling this function.
    static bool s_is_window_class_registered = false;
    if (!s_is_window_class_registered) {
        WNDCLASSA window_class = {};
        window_class.hInstance = GetModuleHandle(nullptr);
        window_class.lpszClassName = window_class_name;
        window_class.lpfnWndProc = platform_windows_window_procedure;
        RegisterClassA(&window_class);

        s_is_window_class_registered = true;
    }

    // NOTE: Allocate the window object from a linear arena. As the window is most likely to persist for the entire
    //       application lifetime, using a linear allocator is the most efficient.
    WindowsWindow* window = static_cast<WindowsWindow*>(core_linear_arena_allocate(arena, sizeof(WindowsWindow)));

    constexpr DWORD window_style = WS_OVERLAPPEDWINDOW;
    window->handle = CreateWindowA(
        window_class_name,
        "ocean @ AVR-Powerhouse",
        window_style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        nullptr,
        nullptr,
        GetModuleHandle(nullptr),
        nullptr
    );

    // NOTE: There is no point in trying to recover from not having a valid window. The application
    //       is useless without one.
    // TODO: Maybe try again instead of crashing?
    VERIFY(window->handle);

    ShowWindow(window->handle, SW_MAXIMIZE);
    return window;
}

void platform_window_destroy(Window* window)
{
    WindowsWindow* windows_window = static_cast<WindowsWindow*>(*window);
    DestroyWindow(windows_window->handle);
    windows_window->handle = nullptr;

    // NOTE: This is not a memory leak, as the window object is allocated by a linear arena, which
    //       can't release individual allocations anyway.
    *window = nullptr;
}

bool platform_window_get_message(Window window)
{
    const WindowsWindow* windows_window = static_cast<WindowsWindow*>(window);
    VERIFY(windows_window->handle);

    MSG message = {};
    const bool should_contiune_to_run = GetMessageA(&message, windows_window->handle, 0, 0);
    if (should_contiune_to_run) {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }

    return should_contiune_to_run;
}

u32 platform_window_width(Window window)
{
    const WindowsWindow* windows_window = static_cast<WindowsWindow*>(window);
    VERIFY(windows_window->handle);

    RECT window_rect = {};
    GetClientRect(windows_window->handle, &window_rect);
    return (window_rect.right - window_rect.left);
}

u32 platform_window_height(Window window)
{
    const WindowsWindow* windows_window = static_cast<WindowsWindow*>(window);
    VERIFY(windows_window->handle);

    RECT window_rect = {};
    GetClientRect(windows_window->handle, &window_rect);
    return (window_rect.bottom - window_rect.top);
}

i32 platform_window_position_x(Window window)
{
    const WindowsWindow* windows_window = static_cast<WindowsWindow*>(window);
    VERIFY(windows_window->handle);

    RECT window_rect = {};
    GetClientRect(windows_window->handle, &window_rect);
    return window_rect.left;
}

i32 platform_window_position_y(Window window)
{
    const WindowsWindow* windows_window = static_cast<WindowsWindow*>(window);
    VERIFY(windows_window->handle);

    RECT window_rect = {};
    GetClientRect(windows_window->handle, &window_rect);
    return window_rect.top;
}

static LRESULT platform_windows_window_procedure(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param)
{
    switch (message) {
        case WM_CLOSE: {
            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProcA(window_handle, message, w_param, l_param);
}
