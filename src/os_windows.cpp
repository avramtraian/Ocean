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
#include <stdint.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/ftlcdfil.h>

#define internal         static
#define function         static
#define local_persistent static
#define constant         static constexpr

#define BIT(x)           (1 << (x))
#define ARRAY_COUNT(x)   (sizeof(x) / sizeof((x)[0]))

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

struct OSRingBuffer {
    u8* data;
    usize size;
    usize used;
    usize head;
    usize tail;
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

internal OSRingBuffer       os_allocate_ring_buffer         (usize size);
internal void               os_free_ring_buffer             (OSRingBuffer* buffer);
internal void*              os_allocate_from_ring_buffer    (OSRingBuffer* buffer, usize size, usize alignment);
internal void               os_pop_oldest_from_ring_buffer  (OSRingBuffer* buffer, usize size);
internal void               os_pop_newest_from_ring_buffer  (OSRingBuffer* buffer, usize size);
internal void               os_reset_ring_buffer            (OSRingBuffer* buffer);

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
#include "config.cpp"
#include "state.cpp"
#include "navigation.cpp"
#include "insertion.cpp"
#include "gather_render_data.cpp"
#include "render.cpp"

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

typedef PVOID(WINAPI PFN_VirtualAlloc2)(HANDLE, PVOID, SIZE_T, ULONG, ULONG, MEM_EXTENDED_PARAMETER*, ULONG);
typedef PVOID(WINAPI PFN_MapViewOfFile3)(HANDLE, HANDLE, PVOID, ULONG64, SIZE_T, ULONG, ULONG, MEM_EXTENDED_PARAMETER*, ULONG);

internal OSRingBuffer
os_allocate_ring_buffer(usize size)
{
    local_persistent PFN_VirtualAlloc2* VirtualAlloc2 = NULL;
    local_persistent PFN_MapViewOfFile3* MapViewOfFile3 = NULL;
    if (VirtualAlloc2 == NULL || MapViewOfFile3 == NULL) {
        HMODULE kernel32 = GetModuleHandle("kernelbase.dll");
        VirtualAlloc2  = (PFN_VirtualAlloc2* )GetProcAddress(kernel32, "VirtualAlloc2");
        MapViewOfFile3 = (PFN_MapViewOfFile3*)GetProcAddress(kernel32, "MapViewOfFile3");

        ASSERT(VirtualAlloc2); // Windows version is too old...
        ASSERT(MapViewOfFile3); // Windows version is too old...
    }

    ASSERT(os_is_memory_page_aligned(size));
    HANDLE mapping = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
                                        (DWORD)(size >> 32), (DWORD)(size & 0xFFFFFFFF), NULL);
    ASSERT(mapping);

    // Allocate a large region sufficient to map the buffer contents twice.
    u8* base_address = (u8*)VirtualAlloc2(GetCurrentProcess(), NULL, 2 * size,
                                          MEM_RESERVE | MEM_RESERVE_PLACEHOLDER, PAGE_NOACCESS, NULL, 0);

    // Calling VirtualFree on the first half with MEM_PRESERVE_PLACEHOLDER 
    // splits the single (2*size) placeholder into two adjacent (size) placeholders.
    bool split_result = VirtualFree(base_address, size, MEM_RELEASE | MEM_PRESERVE_PLACEHOLDER);
    ASSERT(split_result);

    MapViewOfFile3(mapping, GetCurrentProcess(), base_address       , 0, size, MEM_REPLACE_PLACEHOLDER, PAGE_READWRITE, NULL, 0);
    MapViewOfFile3(mapping, GetCurrentProcess(), base_address + size, 0, size, MEM_REPLACE_PLACEHOLDER, PAGE_READWRITE, NULL, 0);

    CloseHandle(mapping);

    OSRingBuffer result = {};
    result.data = base_address;
    result.size = size;
    result.used = 0;
    result.head = 0;
    result.tail = 0;
    return result;
}

internal void
os_free_ring_buffer(OSRingBuffer* buffer)
{
    // @Leak!
    ZERO_STRUCT_POINTER(buffer);
}

internal void*
os_allocate_from_ring_buffer(OSRingBuffer* buffer, usize size, usize alignment)
{
    uintptr unaligned_address = (uintptr)buffer->data + buffer->head;
    uintptr aligned_address = align_to_pow2(unaligned_address, alignment); // Should always be a power of 2.
    usize alignment_offset = aligned_address - unaligned_address;
    usize total_size = alignment_offset + size;

    if (buffer->used + total_size > buffer->size) {
        // It's impossible to allocate a block this big.
        return NULL;
    }

    usize size_mask = buffer->size - 1; // Always a power of 2.
    buffer->head = (buffer->head + total_size) & size_mask;
    buffer->used += total_size;

    void* address = (void*)aligned_address;
    zero_memory(address, size);
    return address;
}

internal void
os_pop_oldest_from_ring_buffer(OSRingBuffer* buffer, usize size)
{
    ASSERT(size <= buffer->used);

    usize size_mask = buffer->size - 1; // Always a power of 2.
    buffer->tail = (buffer->tail + size) & size_mask;
    buffer->used -= size;
}

internal void
os_pop_newest_from_ring_buffer(OSRingBuffer* buffer, usize size)
{
    ASSERT(size <= buffer->used);

    usize size_mask = buffer->size - 1; // Always a power of 2.
    buffer->head = ((buffer->head + buffer->size) - size) & size_mask;
    buffer->used -= size;
}

internal void
os_reset_ring_buffer(OSRingBuffer* buffer)
{
    buffer->used = 0;
    buffer->head = 0;
    buffer->tail = 0;
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

#define WIN32_MAP_VIRTUAL_KEY_TO_KEY_CODE(x)                                    \
    x('A', KeyCode_A) x('B', KeyCode_B) x('C', KeyCode_C) x('D', KeyCode_D)     \
    x('E', KeyCode_E) x('F', KeyCode_F) x('G', KeyCode_G) x('H', KeyCode_H)     \
    x('I', KeyCode_I) x('J', KeyCode_J) x('K', KeyCode_K) x('L', KeyCode_L)     \
    x('M', KeyCode_M) x('N', KeyCode_N) x('O', KeyCode_O) x('P', KeyCode_P)     \
    x('Q', KeyCode_Q) x('R', KeyCode_R) x('S', KeyCode_S) x('T', KeyCode_T)     \
    x('U', KeyCode_U) x('V', KeyCode_V) x('W', KeyCode_W) x('X', KeyCode_X)     \
    x('Y', KeyCode_Y) x('Z', KeyCode_Z)                                         \
                                                                                \
    x('0', KeyCode_Zero)  x('1', KeyCode_One)   x('2', KeyCode_Two)             \
    x('3', KeyCode_Three) x('4', KeyCode_Four)  x('5', KeyCode_Five)            \
    x('6', KeyCode_Six)   x('7', KeyCode_Seven) x('8', KeyCode_Eight)           \
    x('9', KeyCode_Nine)                                                        \
                                                                                \
    x(VK_OEM_3,     KeyCode_Tilde)        x(VK_OEM_MINUS,  KeyCode_Minus)       \
    x(VK_OEM_PLUS,  KeyCode_Equal)        x(VK_OEM_4,      KeyCode_LeftBracket) \
    x(VK_OEM_6,     KeyCode_RightBracket) x(VK_OEM_1,      KeyCode_Semicolon)   \
    x(VK_OEM_7,     KeyCode_Apostrophe)   x(VK_OEM_5,      KeyCode_Backslash)   \
    x(VK_OEM_COMMA, KeyCode_Comma)        x(VK_OEM_PERIOD, KeyCode_Dot)         \
    x(VK_OEM_2,     KeyCode_Slash)                                              \
                                                                                \
    x(VK_SPACE,  KeyCode_Space)     x(VK_TAB,    KeyCode_Tab)                   \
    x(VK_BACK,   KeyCode_Backspace)                                             \
    x(VK_DELETE, KeyCode_Delete)    x(VK_INSERT, KeyCode_Insert)                \
    x(VK_HOME,   KeyCode_Home)      x(VK_END, KeyCode_End)                      \
    x(VK_RETURN, KeyCode_Enter)     x(VK_ESCAPE, KeyCode_Escape)                \
                                                                                \
    x(VK_LEFT,  KeyCode_Left)   x(VK_RIGHT, KeyCode_Right)                      \
    x(VK_UP,    KeyCode_Up)     x(VK_DOWN,  KeyCode_Down)                       \
    x(VK_PRIOR, KeyCode_PageUp) x(VK_NEXT,  KeyCode_PageDown)                   \
                                                                                \
    x(VK_F1,  KeyCode_F1)  x(VK_F2,  KeyCode_F2)  x(VK_F3,  KeyCode_F3)         \
    x(VK_F4,  KeyCode_F4)  x(VK_F5,  KeyCode_F5)  x(VK_F6,  KeyCode_F6)         \
    x(VK_F7,  KeyCode_F7)  x(VK_F8,  KeyCode_F8)  x(VK_F9,  KeyCode_F9)         \
    x(VK_F10, KeyCode_F10) x(VK_F11, KeyCode_F11) x(VK_F12, KeyCode_F12)        \
                                                                                \
    x(VK_SHIFT, KeyCode_Shift) x(VK_CONTROL, KeyCode_Control)                   \
    x(VK_MENU, KeyCode_Alt)

internal KeyCode
win32_key_code_from_virtual_key(int virtual_key)
{
    switch (virtual_key) {
#define WIN32_MAPPING(virtual_key_value, key_code) case virtual_key_value: return key_code;
      WIN32_MAP_VIRTUAL_KEY_TO_KEY_CODE(WIN32_MAPPING);
#undef WIN32_MAPPING

      default: return KeyCode_Unknown; // Unknown VK.
    }
}

internal int
win32_virtual_key_from_key_code(KeyCode key_code)
{
    switch (key_code) {
#define WIN32_MAPPING(virtual_key, key_code_value) case key_code_value: return virtual_key;
      WIN32_MAP_VIRTUAL_KEY_TO_KEY_CODE(WIN32_MAPPING);
#undef WIN32_MAPPING

      default: return 0; // Unknown key code.
    }
}

internal FrameInput g_frame_input;

internal LRESULT
win32_window_procedure(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param)
{
    switch (message) {
      case WM_QUIT:
      case WM_CLOSE: {
        g_window_should_close = true;
        return 0;
      }

      case WM_KEYDOWN:
      case WM_SYSKEYDOWN: {
        int virtual_key = w_param;
        KeyCode key_code = win32_key_code_from_virtual_key(virtual_key);
        g_frame_input.keys[key_code].event_count++;
        return 0;
      }

      case WM_CHAR: {
        int codepoint = w_param;
        if (codepoint < ' ' || codepoint == 0x7F) return 0; // Ignore non-printable ASCII codepoints. // @Incomplete: This doesn't exclude all non-printable ASCII characters...

        if (g_frame_input.char_event_count < g_frame_input.max_char_event_count) {
            g_frame_input.char_events[g_frame_input.char_event_count] = codepoint;
            g_frame_input.char_event_count++;
        } else {
            // @Incomplete: Skipping key pressed events when typing is the last thing a text editor should
            // do. However, the value of 'max_char_event_codepoints' is currently quite big...
        }
        return 0;
      }
    }

    return DefWindowProcA(window_handle, message, w_param, l_param);
}

internal void
win32_reset_frame_input()
{
    for (KeyCode key_code = KeyCode_Unknown;
         key_code < KeyCode_MaxEnumCount;
         key_code = (KeyCode)(key_code + 1))
    {
        KeyState* key_state = &g_frame_input.keys[key_code];
        key_state->was_pressed_this_frame = false;
        key_state->was_released_this_frame = false;
        key_state->event_count = 0;
    }

    g_frame_input.char_event_count = 0;
}

internal void
win32_query_frame_input()
{
    HWND focused_window = GetFocus();
    bool window_has_focus = (focused_window == g_window_handle);

    for (KeyCode key_code = KeyCode_Unknown;
         key_code < KeyCode_MaxEnumCount;
         key_code = (KeyCode)(key_code + 1))
    {
        KeyState* key_state = &g_frame_input.keys[key_code];
        bool was_previously_down = key_state->is_down;
        key_state->is_down = false;

        // NOTE(Traian): This ensures that when losing window focus (for whatever reason) all keys that are
        // currently down will have the 'was_released_this_frame' flag set to true during this frame. If we
        // however regain focus without the key being released, we will set the 'was_pressed_this_frame' flag,
        // but the window procedure will not set 'received_key_down_event'. This inconsistency might be a problem,
        // but fixing it would introduce quite a bit of complexity in input state management. (9th January 2026)

        if (window_has_focus) {
            int virtual_key = win32_virtual_key_from_key_code(key_code);
            SHORT win32_key_state = GetKeyState(virtual_key);
            if (win32_key_state & (1 << 15))
                key_state->is_down = true;
        }

        if (key_state->is_down && !was_previously_down)
            key_state->was_pressed_this_frame = true;

        if (!key_state->is_down && was_previously_down)
            key_state->was_released_this_frame = true;
    }
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
    // fonts_description.text_name     = "../DroidSansMono.ttf";
    // fonts_description.ui_name       = "../DroidSansMono.ttf";
    // fonts_description.ui_small_name = "../DroidSansMono.ttf";
    // fonts_description.ui_big_name   = "../DroidSansMono.ttf";
    fonts_description.text_name     = "C:/Windows/Fonts/consola.ttf";
    fonts_description.ui_name       = "C:/Windows/Fonts/consola.ttf";
    fonts_description.ui_small_name = "C:/Windows/Fonts/consola.ttf";
    fonts_description.ui_big_name   = "C:/Windows/Fonts/consola.ttf";
    fonts_description.text_pixel_height     = 23;
    fonts_description.ui_pixel_height       = 25;
    fonts_description.ui_small_pixel_height = 15;
    fonts_description.ui_big_pixel_height   = 30;
    
    MemoryArena eternal_arena = create_arena(KiB(4), MiB(1));
    MemoryArena frame_arena   = create_arena(MiB(16), GiB(1));
    MemoryArena fonts_arena   = create_arena(KiB(128), MiB(64));
    g_arenas.eternal = &eternal_arena;
    g_arenas.frame = &frame_arena;
    g_arenas.fonts = &fonts_arena;

    reload_global_fonts(&fonts_description);

    EditorState editor_state = {};
    editor_state.active_panel = &editor_state.first_panel;
    editor_state.first_panel.wrap_content_lines = true;
    EditorBuffer* buffer = &editor_state.first_panel.content_buffer;
    buffer->reserved = MiB(16);
    buffer->committed = MiB(16);
    buffer->data = (u8*)os_allocate_memory(buffer->committed);

    OSReadFileResult read = os_read_entire_file("C:/Dev/editor3/src/os_windows.cpp");
    if (read.is_valid) {
        copy_memory(buffer->data, read.data, read.size);
        buffer->size = read.size;
    }

    buffer->cursor_allocated_count = 16;
    buffer->cursors = PUSH_ARRAY(g_arenas.eternal, EditorCursor, buffer->cursor_allocated_count);
    buffer->cursor_count = 1;
    buffer->cursors[0].tail_offset = 0;
    buffer->cursors[0].head_offset = 25;

    g_window_should_close = false;
    while (!g_window_should_close) {
        // Process the message queue.
        win32_reset_frame_input();
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

        win32_query_frame_input();

        if (g_frame_input.keys[KeyCode_Control].is_down) {
            s32 pixel_height = g_fonts.text.pixel_height;
            if (g_frame_input.keys[KeyCode_Minus].event_count > 0)
                pixel_height = clamp(pixel_height - 1, 1, 100);
            if (g_frame_input.keys[KeyCode_Equal].event_count > 0)
                pixel_height = clamp(pixel_height + 1, 1, 100);

            if (pixel_height != g_fonts.text.pixel_height) {
                reset_memory_arena(g_arenas.fonts);
                fonts_description.text_pixel_height = pixel_height;
                reload_global_fonts(&fonts_description);
            }
        }

        reset_memory_arena(g_arenas.frame);
        update_navigation_system(&editor_state, &g_frame_input);
        update_insertion_system(&editor_state, &g_frame_input);
        gather_render_data(&editor_state);
        draw_editor_frame(&editor_state);

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
