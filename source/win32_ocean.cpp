/*  =====================================================================
    $File:   win32_ocean.cpp $
    $Date:   September 17 2023 $
    $Author: Traian Avram $
    $Notice: Copyright (c) 2023-2023 Traian Avram. All Rights Reserved. $
    =====================================================================  */

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <debugapi.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long long s64;

typedef s8 b8;
typedef s32 b32;
typedef float f32;
typedef double f64;

typedef u64 memory_size;
typedef u64 memory_offset;
typedef u64 usize;
typedef u64 flat_ptr;

#include "ocean.cpp"

struct win32_offscreen_bitmap
{
    BITMAPINFO Info;
    u32 Width;
    u32 Height;
    // NOTE(traian): A bitmap pixel is always 32-bit.
    u32 *Pixels;
};

global win32_offscreen_bitmap GlobalOffscreenBitmap;

internal void
Win32ResizeOffscreenBitmap(u32 Width, u32 Height)
{
    if (GlobalOffscreenBitmap.Pixels)
    {
        VirtualFree(GlobalOffscreenBitmap.Pixels, 0, MEM_RELEASE);
        GlobalOffscreenBitmap.Pixels = NULL;
    }

    GlobalOffscreenBitmap.Width = Width;
    GlobalOffscreenBitmap.Height = Height;

    GlobalOffscreenBitmap.Info.bmiHeader.biSize = sizeof(GlobalOffscreenBitmap.Info.bmiHeader);
    GlobalOffscreenBitmap.Info.bmiHeader.biWidth = Width;
    GlobalOffscreenBitmap.Info.bmiHeader.biHeight = Height;
    GlobalOffscreenBitmap.Info.bmiHeader.biPlanes = 1;
    GlobalOffscreenBitmap.Info.bmiHeader.biBitCount = 32;
    GlobalOffscreenBitmap.Info.bmiHeader.biCompression = BI_RGB;

    memory_size BitmapByteCount = Width * Height * 4;
    GlobalOffscreenBitmap.Pixels = (u32 *)VirtualAlloc(0, BitmapByteCount, MEM_COMMIT, PAGE_READWRITE);
    Assert(GlobalOffscreenBitmap.Pixels);
}

internal void
Win32UpdateWindowBitmap(HDC DeviceContext, u32 ClientWidth, u32 ClientHeight)
{
    StretchDIBits(DeviceContext,
                  0, 0, ClientWidth, ClientHeight,
                  0, 0, GlobalOffscreenBitmap.Width, GlobalOffscreenBitmap.Height,
                  GlobalOffscreenBitmap.Pixels, &GlobalOffscreenBitmap.Info,
                  DIB_RGB_COLORS, SRCCOPY);
}

global WINDOWPLACEMENT GlobalPreviousWindowPlacement;

internal void
Win32ToggleFullscreen(HWND WindowHandle)
{
    DWORD WindowStyle = GetWindowLong(WindowHandle, GWL_STYLE);
    if (WindowStyle & WS_OVERLAPPEDWINDOW)
    {
        MONITORINFO MonitorInfo = { sizeof(MonitorInfo) };
        if (GetWindowPlacement(WindowHandle, &GlobalPreviousWindowPlacement) &&
            GetMonitorInfo(MonitorFromWindow(WindowHandle, MONITOR_DEFAULTTONEAREST), &MonitorInfo))
        {
            SetWindowLong(WindowHandle, GWL_STYLE, WindowStyle & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(WindowHandle, HWND_TOP,
                         MonitorInfo.rcMonitor.left, MonitorInfo.rcMonitor.top,
                         MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left,
                         MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    else
    {
        SetWindowLong(WindowHandle, GWL_STYLE, WindowStyle | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(WindowHandle, &GlobalPreviousWindowPlacement);
        SetWindowPos(WindowHandle, NULL, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

global editor_memory GlobalEditorMemory;
global editor_state *GlobalEditorState;
global HWND GlobalWindowHandle;

internal key_code
Win32TranslateKeyCode(WPARAM WParam)
{
    if ('A' <= WParam && WParam <= 'Z')
    {
        return (key_code)WParam;
    }
    if ('0' <= WParam && WParam <= '9')
    {
        return (key_code)(KeyCode_Zero + (WParam - '0'));
    }
    if (VK_F1 <= WParam && WParam <= VK_F12)
    {
        return (key_code)(KeyCode_FKeyFirst + (WParam - VK_F1));
    }

    switch (WParam)
    {
        case VK_LEFT:       return KeyCode_ArrowLeft;
        case VK_RIGHT:      return KeyCode_ArrowRight;
        case VK_UP:         return KeyCode_ArrowUp;
        case VK_DOWN:       return KeyCode_ArrowDown;
        case VK_PRIOR:      return KeyCode_PageUp;
        case VK_NEXT:       return KeyCode_PageDown;
        case VK_BACK:       return KeyCode_Backspace;
        case VK_DELETE:     return KeyCode_Delete;
        case VK_SPACE:      return KeyCode_Space;
        case VK_RETURN:     return KeyCode_Enter;
        case VK_TAB:        return KeyCode_Tab;

        case VK_OEM_1:      return KeyCode_Semicolon;
        case VK_OEM_PLUS:   return KeyCode_Equals;
        case VK_OEM_COMMA:  return KeyCode_Comma;
        case VK_OEM_MINUS:  return KeyCode_Minus;
        case VK_OEM_PERIOD: return KeyCode_Period;
        case VK_OEM_2:      return KeyCode_Slash;
        case VK_OEM_3:      return KeyCode_Backtick;
        case VK_OEM_4:      return KeyCode_LeftBracket;
        case VK_OEM_5:      return KeyCode_Backslash;
        case VK_OEM_6:      return KeyCode_RightBracket;
        case VK_OEM_7:      return KeyCode_Apostrophe;
    }

    return KeyCode_Invalid;
}

internal void
UpdateEditor(HWND WindowHandle)
{
    bitmap OffscreenBitmap = {};
    OffscreenBitmap.Width = GlobalOffscreenBitmap.Width;
    OffscreenBitmap.Height = GlobalOffscreenBitmap.Height;
    OffscreenBitmap.BytesPerPixel = 4;
    OffscreenBitmap.Pitch = OffscreenBitmap.Width * OffscreenBitmap.BytesPerPixel;
    OffscreenBitmap.Memory = (u8 *)GlobalOffscreenBitmap.Pixels;

    UpdateAndRenderEditor(GlobalEditorState, &GlobalEditorMemory, &OffscreenBitmap);
    Win32UpdateWindowBitmap(GetDC(WindowHandle), GlobalEditorState->WindowWidth, GlobalEditorState->WindowHeight);
}

internal LRESULT CALLBACK
WindowProcedure(HWND WindowHandle, UINT MessageID, WPARAM WParam, LPARAM LParam)
{
    switch (MessageID)
    {
        case WM_CLOSE:
        {
            PostQuitMessage(0);
            return 0;
        }

        case WM_SIZE:
        {
            u32 Width = LOWORD(LParam);
            u32 Height = HIWORD(LParam);

            if (Width > 0 && Height > 0)
            {
                if (GlobalEditorState)
                {
                    if (GlobalEditorState->WindowWidth != Width || GlobalEditorState->WindowHeight != Height)
                    {
                        Win32ResizeOffscreenBitmap(Width, Height);
                        
                        GlobalEditorState->WindowWidth = Width;
                        GlobalEditorState->WindowHeight = Height;
                        UpdateEditor(WindowHandle);
                    }
                }
                else
                {
                    Win32ResizeOffscreenBitmap(Width, Height);
                }
            }
            
            return 0;
        }

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        {
            key_code KeyCode = Win32TranslateKeyCode(WParam);
            if (KeyCode != KeyCode_Invalid)
            {
                EditorEventKeyPressed(GlobalEditorState, KeyCode);
                UpdateEditor(WindowHandle);
            }

            return 0;
        }

        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            return 0;
        }

        case WM_SYSCOMMAND:
        {
            if (WParam == SC_KEYMENU)
            {
                // NOTE(traian): Disable the annoying beeping sound when pressing Alt+Key.
                // Why did Windows consider this neccessary?
                return 0;
            }
        } break;
    }

    return DefWindowProcA(WindowHandle, MessageID, WParam, LParam);
}

INT
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CommandLine,
        int ShowCommand)
{
    WNDCLASSA WindowClass = {};
    WindowClass.lpfnWndProc = WindowProcedure;
    WindowClass.hInstance = Instance;
    WindowClass.hIcon = LoadIconA(Instance, "MAINICON");
    WindowClass.lpszClassName = "OceanWindowClass";
    RegisterClassA(&WindowClass);

    HWND WindowHandle = CreateWindowA("OceanWindowClass",
                                      "ocean @ AVR-PowerHouse",
                                      WS_OVERLAPPEDWINDOW | WS_MAXIMIZE,
                                      CW_USEDEFAULT, CW_USEDEFAULT,
                                      CW_USEDEFAULT, CW_USEDEFAULT,
                                      NULL, NULL, Instance, NULL);
    if (WindowHandle)
    {
        GlobalWindowHandle = WindowHandle;
        ShowWindow(WindowHandle, SW_MAXIMIZE);

        GlobalEditorMemory.PermanentStorageSize = Megabytes(2);
        GlobalEditorMemory.PermanentStorage = VirtualAlloc(0, GlobalEditorMemory.PermanentStorageSize,
                                                           MEM_COMMIT, PAGE_READWRITE);
        InitializeArena(&GlobalEditorMemory.PermanentArena,
                        GlobalEditorMemory.PermanentStorage, GlobalEditorMemory.PermanentStorageSize);

        GlobalEditorState = PushStruct(&GlobalEditorMemory.PermanentArena, editor_state);

        RECT WindowClientRect;
        GetClientRect(WindowHandle, &WindowClientRect);
        GlobalEditorState->WindowWidth = WindowClientRect.right - WindowClientRect.left;
        GlobalEditorState->WindowHeight = WindowClientRect.bottom - WindowClientRect.top;

        HDC DeviceContext = GetDC(WindowHandle);
        Win32ResizeOffscreenBitmap(GlobalEditorState->WindowWidth, GlobalEditorState->WindowHeight);

        InitializeEditor(GlobalEditorState, &GlobalEditorMemory);
        UpdateEditor(WindowHandle);

        MSG Message;
        while (GetMessageA(&Message, NULL, 0, 0) > 0)
        {
            TranslateMessage(&Message);
            DispatchMessageA(&Message);
        }
    }
    else
    {
        // TODO(traian): Logging.
        return 1;
    }

    return 0;
}

buffer
PlatformAllocateMemory(memory_size Size)
{
    buffer Block = {};
    if (Size == 0)
    {
        return Block;
    }

    Block.Size = Size;
    Block.Data = (u8 *)VirtualAlloc(0, Size, MEM_COMMIT, PAGE_READWRITE);
    Assert(Block.Data);
    return Block;
}

void
PlatformReleaseMemory(buffer Block)
{
    if (Block.Data != NULL && Block.Size > 0)
    {
        VirtualFree(Block.Data, Block.Size, MEM_RELEASE);
    }
}

memory_size
PlatformGetFileSize(char *FileName)
{
    HANDLE FileHandle = CreateFileA(FileName, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                                    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (FileHandle == INVALID_HANDLE_VALUE)
    {
        // TODO(traian): Logging.
        return INVALID_SIZE;
    }

    LARGE_INTEGER FileSize;
    BOOL Success = GetFileSizeEx(FileHandle, &FileSize);
    Assert(Success);

    CloseHandle(FileHandle);
    return FileSize.QuadPart;
}

internal HANDLE
Win32OpenFileForReading(char *FileName)
{
    HANDLE FileHandle = CreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, NULL,
                                    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    return FileHandle;
}

internal memory_size
Win32ReadFileToBuffer(HANDLE FileHandle, buffer Buffer)
{
    DWORD BytesRead;
    if (!ReadFile(FileHandle, Buffer.Data, Buffer.Size, &BytesRead, NULL))
    {
        return INVALID_SIZE;
    }
    return BytesRead;
}

memory_size
PlatformReadEntireFile(char *FileName, buffer FileBuffer)
{
    HANDLE FileHandle = Win32OpenFileForReading(FileName);
    if (FileHandle == INVALID_HANDLE_VALUE)
    {
        // TODO(traian): Logging.
        return 0;
    }

    LARGE_INTEGER FileSize;
    BOOL Success = GetFileSizeEx(FileHandle, &FileSize);
    Assert(Success);

    Assert(FileBuffer.Size >= FileSize.QuadPart);
    memory_size ByteCount = Win32ReadFileToBuffer(FileHandle, FileBuffer);
    Assert(ByteCount != INVALID_SIZE);

    CloseHandle(FileHandle);
    return ByteCount;
}


buffer
PlatformReadEntireFile(char *FileName, memory_arena *Arena)
{
    HANDLE FileHandle = Win32OpenFileForReading(FileName);
    if (FileHandle == INVALID_HANDLE_VALUE)
    {
        // TODO(traian): Logging.
        return {};
    }

    LARGE_INTEGER FileSize;
    BOOL Success = GetFileSizeEx(FileHandle, &FileSize);
    Assert(Success);

    buffer FileBuffer;
    FileBuffer.Size = FileSize.QuadPart;
    FileBuffer.Data = PushSize(Arena, FileBuffer.Size);

    memory_size ByteCount = Win32ReadFileToBuffer(FileHandle, FileBuffer);
    Assert(ByteCount == FileBuffer.Size);

    CloseHandle(FileHandle);
    return FileBuffer;
}

memory_size
PlatformWriteEntireFile(char *FileName, buffer Buffer)
{
    HANDLE FileHandle = CreateFileA(FileName, GENERIC_WRITE, 0, NULL,
                                    TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (FileHandle == INVALID_HANDLE_VALUE)
    {
        // NOTE(traian): The file doesn't exist on disk.
        HANDLE FileHandle = CreateFileA(FileName, GENERIC_WRITE, 0, NULL,
                                        OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        
        if (FileHandle == INVALID_HANDLE_VALUE)
        {
            // TODO(traian): Logging.
            return 0;
        }
    }

    DWORD BytesWritten;
    WriteFile(FileHandle, Buffer.Data, Buffer.Size, &BytesWritten, NULL);
    if (BytesWritten != Buffer.Size)
    {
        // TODO(traian): Logging.
        return 0;
    }

    CloseHandle(FileHandle);
    return BytesWritten;
}

key_modifier
PlatformGetKeyModifiers()
{
    u8 Result = KeyModifier_None;

    if (GetKeyState(VK_LCONTROL) & 0x8000 || GetKeyState(VK_RCONTROL) & 0x8000)
    {
        Result |= KeyModifier_Ctrl;
    }
    if (GetKeyState(VK_LSHIFT) & 0x8000 || GetKeyState(VK_RSHIFT) & 0x8000)
    {
        Result |= KeyModifier_Shift;
    }
    if (GetKeyState(VK_LMENU) & 0x8000 || GetKeyState(VK_RMENU) & 0x8000)
    {
        Result |= KeyModifier_Alt;
    }

    return (key_modifier)Result;
}

b32
PlatformIsCapsLockActive()
{
    b32 Result = (GetKeyState(VK_CAPITAL) & 1);
    return Result;
}

void
PlatformQuit()
{
    PostQuitMessage(0);
}

void
PlatformToggleFullscreen()
{
    Win32ToggleFullscreen(GlobalWindowHandle);
}
