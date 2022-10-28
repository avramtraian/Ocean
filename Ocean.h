/**
 *--------------------------------------------------------------------------------------
 * Ocean.h
 *--------------------------------------------------------------------------------------
 * Ocean is a framework that aims towards creating a platform that can
 *   used for creating performance-critical demos & presentations, while
 *   having a simple and descriptive API.
 * 
 * The GitHub project can be found at: 'https://github.com/avramtraian/Ocean'.
 */

/**
 * MIT License
 * 
 * Copyright(c) 2022-2022 Avram Traian
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files(the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions :
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#ifndef OC_IMPLEMENTATION
    #define OC_IMPLEMENTATION                       0
#endif // OC_IMPLEMENTATION

#ifdef _WIN32
    #define OC_PLATFORM_WINDOWS                     1
    #ifdef _WIN64
        #define OC_PLATFORM_WINDOWS_64              1
    #else
        #define OC_PLATFORM_WINDOWS_32              1
    #endif // _WIN64
#endif // _WIN32

#ifndef OC_PLATFORM_WINDOWS
    #define OC_PLATFORM_WINDOWS                     0
#endif // OC_PLATFORM_WINDOWS

#ifndef OC_PLATFORM_WINDOWS_32
    #define OC_PLATFORM_WINDOWS_32                  0
#endif // OC_PLATFORM_WINDOWS_32

#ifndef OC_PLATFORM_WINDOWS_64
    #define OC_PLATFORM_WINDOWS_64                  0
#endif // OC_PLATFORM_WINDOWS_64

#if !OC_PLATFORM_WINDOWS && !OC_PLATFORM_LINUX && !OC_PLATFORM_MACOS
    #error Unknown or unsupported platform! Ocean can only be used on Windows.
#endif

#ifdef _MSC_BUILD
    #define OC_COMPILER_MSVC                        1
#endif // _MSC_BUILD

#ifdef __clang__
    #define OC_COMPILER_CLANG                       1
    #define OC_COMPILER_CLANG_GCC                   1
#endif // __clang__

#ifdef __gcc__
    #define OC_COMPILER_GCC                         1
    #define OC_COMPILER_CLANG_GCC                   1
#endif // __gcc__

#ifndef OC_COMPILER_MSVC
    #define OC_COMPILER_MSVC                        0
#endif // OC_COMPILER_MSVC
#ifndef OC_COMPILER_CLANG
    #define OC_COMPILER_CLANG                       0
#endif // OC_COMPILER_CLANG
#ifndef OC_COMPILER_GCC
    #define OC_COMPILER_GCC                         0
#endif // OC_COMPILER_GCC
#ifndef OC_COMPILER_CLANG_GCC
    #define OC_COMPILER_CLANG_GCC                   0
#endif // OC_COMPILER_CLANG_GCC

#if !OC_COMPILER_MSVC && !OC_COMPILER_CLANG && !OC_COMPILER_GCC
    #error Unknown or unsupported compiler! Ocean can only be compiled with MSVC, Clang or GCC.
#endif

#define OC_CONFIGURATION_ALREADY_DEFINED            0

#ifdef OC_DEBUG
    #if OC_DEBUG
        #undef OC_CONFIGURATION_ALREADY_DEFINED
        #define OC_CONFIGURATION_ALREADY_DEFINED    1
    #endif // OC_DEBUG
#endif // OC_DEBUG
#ifdef OC_RELEASE
    #if OC_RELEASE
        #undef OC_CONFIGURATION_ALREADY_DEFINED
        #define OC_CONFIGURATION_ALREADY_DEFINED    1
    #endif // OC_RELEASE
#endif // OC_RELEASE

#if !OC_CONFIGURATION_ALREADY_DEFINED
    #ifdef _DEBUG
        #define OC_DEBUG                            1
    #else
        #define OC_RELEASE                          1
    #endif // _DEBUG
#endif // !OC_CONFIGURATION_ALREADY_DEFINED

#undef OC_CONFIGURATION_ALREADY_DEFINED

#ifndef OC_DEBUG
    #define OC_DEBUG                                0
#endif // OC_DEBUG
#ifndef OC_RELEASE
    #define OC_RELEASE                              0
#endif // OC_RELEASE

#if !OC_DEBUG && !OC_RELEASE
    #error No build configuration was specified!
#endif

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#if OC_PLATFORM_WINDOWS
#include <Windows.h>
#endif // OC_PLATFORM_WINDOWS

#define internal    static
#define persistent  static

#define true    (1)
#define false   (0)

typedef int8_t      s8;
typedef int16_t     s16;
typedef int32_t     s32;
typedef int64_t     s64;

typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;

typedef float       r32;
typedef double      r64;

typedef int8_t      b8;
typedef int32_t     b32;

#define SetStringToBuffer(STRING, BUFFER) MemoryCopy(BUFFER, STRING, sizeof(STRING))

void MemoryCopy(void* Destination, const void* Source, u64 Size);

typedef struct game_create_data
{
    char WindowTitle[256];
    u32 WindowWidth;
    u32 WindowHeight;
    s32 WindowPositionX;
    s32 WindowPositionY;
} game_create_data;

void OnCreate(game_create_data* GameCreateData);

void OnUpdate(float DeltaTime);

void OnDestroy();

#if OC_IMPLEMENTATION

#define Kilobytes(X) (X * 1024ULL)
#define Megabytes(X) (Kilobytes(X) * 1024ULL)
#define Gigabytes(X) (Megabytes(X) * 1024ULL)

void MemoryCopy(void* Destination, const void* Source, u64 Size)
{
    memcpy(Destination, Source, (size_t)Size);
}

typedef struct linear_memory_arena
{
    void*   BaseAddress;
    u64     AllocatedOffset;
    u64     BlockSize;
} linear_memory_arena;

internal b8 CreateLinearMemoryArena(linear_memory_arena** MemoryArena, u64 ArenaSize)
{
    if (MemoryArena == NULL)
    {
        return (false);
    }
    if (ArenaSize == 0)
    {
        return (false);
    }

    u64 MemoryRequirement = sizeof(linear_memory_arena) + ArenaSize;
    void* Memory = malloc(MemoryRequirement);
    if (Memory == NULL)
    {
        return (false);
    }

    *MemoryArena = (linear_memory_arena*)Memory;
    linear_memory_arena* Arena = *MemoryArena;

    Arena->BaseAddress = ((u8*)Memory) + sizeof(linear_memory_arena);
    Arena->AllocatedOffset = 0;
    Arena->BlockSize = ArenaSize;

    return (true);
}

internal void DestroyLinearMemoryArena(linear_memory_arena** MemoryArena)
{
    if (MemoryArena == NULL || *MemoryArena == NULL)
    {
        return;
    }

    free((*MemoryArena)->BaseAddress);
    *MemoryArena = NULL;
}

internal void* AllocateLinearMemoryArena(linear_memory_arena* MemoryArena, u64 BlockSize)
{
    if (MemoryArena == NULL)
    {
        return (NULL);
    }
    if (BlockSize == 0)
    {
        return (NULL);
    }

    if (MemoryArena->AllocatedOffset + BlockSize > MemoryArena->BlockSize)
    {
        // TODO(Avr): Logging.
        return (NULL);
    }

    void* Memory = ((u8*)MemoryArena->BaseAddress) + MemoryArena->AllocatedOffset;
    MemoryArena->AllocatedOffset += BlockSize;
    return (Memory);
}

internal void ResetLinearMemoryArena(linear_memory_arena* MemoryArena)
{
    if (MemoryArena == NULL)
    {
        return;
    }

    MemoryArena->AllocatedOffset = 0;
}

typedef struct window_data
{
    char    Title[256];
    u32     Width;
    u32     Height;
    s32     PositionX;
    s32     PositionY;
    void*   Handle;
} window_data;

typedef struct game_data
{
    b32             bIsRunning;
    window_data*    WindowData;
} game_data;
internal game_data* GameData;

internal LRESULT OceanWindowProcedure(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam)
{
    switch (Message)
    {
        case WM_CLOSE:
        {
            GameData->bIsRunning = false;
        }
        return (0);
    }

    return DefWindowProc(WindowHandle, Message, WParam, LParam);
}

internal window_data* CreateGameWindow(game_create_data* GameCreateData, linear_memory_arena* MemoryArena)
{
    if (GameCreateData == NULL || MemoryArena == NULL)
    {
        return (NULL);
    }

#if OC_PLATFORM_WINDOWS
    HINSTANCE Instance = GetModuleHandle(NULL);

    WNDCLASSA WindowClass = {};
    WindowClass.lpfnWndProc = OceanWindowProcedure;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = "OceanWindowClass";
    if (!RegisterClassA(&WindowClass))
    {
        // TODO(Avr): Logging.
        return (NULL);
    }

    HWND WindowHandle = CreateWindowA(
        "OceanWindowClass",
        GameCreateData->WindowTitle,
        WS_OVERLAPPEDWINDOW,
        (int)GameCreateData->WindowPositionX, (int)GameCreateData->WindowPositionX,
        (int)GameCreateData->WindowWidth, (int)GameCreateData->WindowHeight,
        NULL, NULL, Instance, NULL
    );

    if (WindowHandle == NULL)
    {
        // TODO(Avr): Logging.
        return (NULL);
    }

    window_data* WindowData = (window_data*)AllocateLinearMemoryArena(MemoryArena, sizeof(window_data));
    WindowData->Handle = WindowHandle;

    MemoryCopy(WindowData->Title, GameCreateData->WindowTitle, sizeof(WindowData->Title));
    WindowData->Width = GameCreateData->WindowWidth;
    WindowData->Height = GameCreateData->WindowHeight;
    WindowData->PositionX = GameCreateData->WindowPositionX;
    WindowData->PositionY = GameCreateData->WindowPositionY;

    ShowWindow(WindowHandle, SW_SHOW);
    return (WindowData);
#endif // OC_PLATFORM_WINDOWS
}

internal void PumpWindowMessages(window_data* WindowData)
{
    MSG Message;
    while (PeekMessageA(&Message, (HWND)WindowData->Handle, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&Message);
        DispatchMessageA(&Message);
    }
}

internal s32 GuardedMain(char** CommandLineArguments, u16 CommandLineArgumentsCount)
{
    game_data GD = {};
    GameData = &GD;

    game_create_data GameCreateData = {};
    OnCreate(&GameCreateData);

    linear_memory_arena* CoreSystemsMemoryArena;
    CreateLinearMemoryArena(&CoreSystemsMemoryArena, Megabytes(1));

    window_data* WindowData = CreateGameWindow(&GameCreateData, CoreSystemsMemoryArena);
    if (WindowData == NULL)
    {
        // TODO(Avr): Logging.
        return -1;
    }

    GameData->bIsRunning = true;
    while (GameData->bIsRunning)
    {
        PumpWindowMessages(WindowData);
        
        float DeltaTime = 0.0F;
        OnUpdate(DeltaTime);
    }

    OnDestroy();
    return (0);
}

#if OC_PLATFORM_WINDOWS

int WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCommand)
{
    return (GuardedMain(__argv, __argc));
}
#endif // OC_PLATFORM_WINDOWS

#endif // OC_IMPLEMENTATION