// Copyright (c) 2026 Traian Avram. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause.

#if PLATFORM_OS_WINDOWS

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <string.h>
#include <stdlib.h>

//==============================================================================
// GENERAL-PURPOSE OS LAYER.
//==============================================================================

INT WINAPI
WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, INT ShowCommand)
{
    int Argc = __argc;
    char** Argv = __argv;

    string8 Arguments[32] = {};
    for (int Index = 0; Index < Argc; ++Index)
    {
        Arguments[Index].Data = (u8*)Argv[Index];
        Arguments[Index].Size = strlen(Argv[Index]);
    }

    s32 ReturnCode = OSEntryPoint(Arguments, Argc);
    return ReturnCode;
}

internal usize
OSGetMemoryPageSize()
{
    local_persistent DWORD PageSize = 0;
    if (PageSize == 0)
    {
        SYSTEM_INFO SystemInfo = {};
        GetSystemInfo(&SystemInfo);
        PageSize = SystemInfo.dwPageSize;
        if (PageSize == 0)
        {
            PANIC("Failed to query the memory page size!");
        }
    }

    return PageSize;
}

internal usize
OSGetMemoryPageAligned(usize UnalignedSize)
{
    usize PageSize = OSGetMemoryPageSize();
    usize Result = RoundUpPowOfTwo(UnalignedSize, PageSize);
    return Result;
}

internal void*
OSVirtualAllocate(void* Address, usize Size, os_allocate_type_bits AllocateType)
{
    if (Size == 0)
        return NULL;

    DWORD Win32AllocationType = 0;
    if (AllocateType & OSAllocateType_Reserve) Win32AllocationType |= MEM_RESERVE;
    if (AllocateType & OSAllocateType_Commit)  Win32AllocationType |= MEM_COMMIT;

    usize AlignedSize = OSGetMemoryPageAligned(Size);
    void* Result = VirtualAlloc(Address, AlignedSize, Win32AllocationType, PAGE_READWRITE);
    return Result;
}

internal b8
OSVirtualFree(void* Address, usize Size, os_free_type FreeType)
{
    if (FreeType == OSFreeType_Release && Size != 0)
    {
        // @Cleanup: Maybe log some kind of warning? We should first see how this API
        // will be abstracted for other platforms. Windows does not allow this parameter
        // to be non-zero when the free type is set to 'Release'. Should we make this
        // a hard error (panic) instead? -- avrtraian 21 May 2026
        Size = 0;
    }

    DWORD Win32FreeType;
    switch (FreeType)
    {
        case OSFreeType_Decommit: Win32FreeType = MEM_DECOMMIT; break;
        case OSFreeType_Release:  Win32FreeType = MEM_RELEASE; break;
        default: PANIC("Invalid free type passed to 'OSVirtualFree'!");
    }

    b8 Result = VirtualFree(Address, Size, FreeType);
    return Result;
}

//==============================================================================
// GRAPHICAL OS LAYER.
//==============================================================================

struct win32_event
{
    win32_event* Next;
    os_event     Event;
};

struct os_window
{
    os_window* Prev;
    os_window* Next;
    HWND       Handle;
    HDC        DeviceContext;
    bitmap     OffscreenBitmap;
};

DECLARE_DLL(win32_window_list,       os_window);
DECLARE_STACK(win32_window_freelist, os_window);
DECLARE_QUEUE(win32_event_queue,     win32_event);
DECLARE_STACK(win32_event_freelist,  win32_event);

struct os_window_manager
{
    arena*                Arena;
    char*                 WindowClassName;
    win32_window_list     Windows;
    win32_window_freelist WindowFreelist;
    win32_event_queue     EventQueue;
    win32_event_freelist  EventFreelist;
};

internal v2u
Win32GetWindowSize(HWND WindowHandle)
{
    RECT ClientRect = {};
    if (!GetClientRect(WindowHandle, &ClientRect))
    {
        return V2u(0, 0);
    }

    v2u Result = {};
    Result.X = ClientRect.right - ClientRect.left;
    Result.Y = ClientRect.bottom - ClientRect.top;
    return Result;
}

internal b8
Win32ReallocateOffscreenBitmap(bitmap* Bitmap, v2u Size)
{
    ZERO_STRUCT(Bitmap);
    if (Size.X == 0 || Size.Y == 0)
    {
        return false;
    }

    usize AllocationSize = cast(usize, Size.X) * cast(usize, Size.Y) * 4;
    void* Pixels = OSVirtualAllocate(NULL, AllocationSize,
                                     OSAllocateType_Reserve | OSAllocateType_Commit);
    if (Pixels == NULL)
    {
        return false;
    }

    Bitmap->Data = cast(u8*, Pixels);
    Bitmap->Width = Size.X;
    Bitmap->Height = Size.Y;
    Bitmap->Format = BitmapFormat_BGRA_8888;
    return true;
}

// @Cleanup: Unfortunately, there is no easy way to avoid global state whean dealing
// with with windows. We must have access to the manager in the window procedure callback
// and this is the most straigth-forward way of achieving that. -- avrtraian 22 May 2026
global_variable os_window_manager* GlobalWindowManager;

internal os_window*
Win32FindWindowByHandle(os_window_manager* Manager, HWND WindowHandle)
{
    DLLForEach(&Manager->Windows, Window)
    {
        if (Window->Handle == WindowHandle)
        {
            return Window;
        }
    }

    return NULL;
}

internal b8
Win32EnqueueEvent(os_window_manager* Manager, os_event* NewEvent)
{
    if (Manager == NULL || NewEvent == NULL)
    {
        return NULL;
    }

    win32_event* Event = NULL;
    StackPop(&Manager->EventFreelist, Event);
    if (Event == NULL)
    {
        Event = PushStruct(Manager->Arena, win32_event);
        if (Event == NULL)
        {
            return NULL;
        }
    }

    QueueEnqueue(&Manager->EventQueue, Event);
    Event->Event = *NewEvent;
    return true;
}

internal LRESULT CALLBACK
Win32WindowProcedure(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam)
{
    switch (Message)
    {
        case WM_QUIT:
        case WM_CLOSE:
        {
            os_window* Window = Win32FindWindowByHandle(GlobalWindowManager, WindowHandle);
            if (Window == NULL)
            {
                break;
            }

            os_event Event = {};
            Event.Type = OSEventType_WindowClosed;
            Event.Window = Window;

            Win32EnqueueEvent(GlobalWindowManager, &Event);
            return 0;
        }

        case WM_SIZE:
        {
            os_window* Window = Win32FindWindowByHandle(GlobalWindowManager, WindowHandle);
            if (Window == NULL)
            {
                break;
            }
            
            os_event Event = {};
            Event.Type = OSEventType_WindowResized;
            Event.Window = Window;
            Event.WindowResized.Width = LOWORD(LParam);
            Event.WindowResized.Height = HIWORD(LParam);

            switch (WParam)
            {
                case SIZE_MAXIMIZED: Event.WindowResized.Mode = OSWindowMode_Maximized; break;
                case SIZE_MINIMIZED: Event.WindowResized.Mode = OSWindowMode_Minimized; break;
                default:             Event.WindowResized.Mode = OSWindowMode_Windowed;  break;
            }

            v2u WindowSize = V2u(Event.WindowResized.Width, Event.WindowResized.Height);
            Win32ReallocateOffscreenBitmap(&Window->OffscreenBitmap, WindowSize);

            Win32EnqueueEvent(GlobalWindowManager, &Event);
            return 0;
        }
    }

    return DefWindowProcA(WindowHandle, Message, WParam, LParam);
}

internal os_window_manager*
OSInitWindowManager(arena* Arena)
{
    os_window_manager* Manager = PushStruct(Arena, os_window_manager);
    if (Manager == NULL)
    {
        return NULL;
    }

    Manager->Arena = Arena;
    Manager->WindowClassName = "Editor5WindowClass";

    WNDCLASSA WindowClass = {};
    WindowClass.style = CS_OWNDC;
    WindowClass.lpfnWndProc = Win32WindowProcedure;
    WindowClass.hInstance = GetModuleHandle(NULL);
    WindowClass.lpszClassName = Manager->WindowClassName;
    if (!RegisterClassA(&WindowClass))
    {
        // @Incomplete: Log error.
        return NULL;
    }

    GlobalWindowManager = Manager;
    return Manager;
}

internal os_window*
OSCreateWindow(os_window_manager* Manager, string8 Title, os_window_mode Mode)
{
    if (Manager == NULL)
    {
        return NULL;
    }

    //
    // Allocate the window object.
    //

    os_window* Window = NULL;
    StackPop(&Manager->WindowFreelist, Window);
    if (Window == NULL)
    {
        Window = PushStruct(Manager->Arena, os_window);
        if (Window == NULL)
        {
            return NULL;
        }
    }

    //
    // Create the native window.
    //

    // @Incomplete @Cleanup @Bugprone: Copy the title to a buffer and make sure it is null
    // terminated! We currently rely that the user will pass a string that was created from
    // a literal, which is obviously very bug-prone!    -- avrtraian 22 May 2026

    HWND WindowHandle = CreateWindowA(Manager->WindowClassName, cast(char*, Title.Data),
                                      WS_OVERLAPPEDWINDOW,
                                      CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                      NULL, NULL, GetModuleHandle(NULL), NULL);
    if (WindowHandle == NULL)
    {
        return NULL;
    }
    ShowWindow(WindowHandle, SW_MAXIMIZE);
    HDC DeviceContext = GetDC(WindowHandle);

    //
    // Allocate the offscreen bitmap.
    //

    v2u WindowSize = Win32GetWindowSize(WindowHandle);
    bitmap OffscreenBitmap = {};
    Win32ReallocateOffscreenBitmap(&OffscreenBitmap, WindowSize);

    //
    // Register the window to the manager.
    //

    Window->Handle = WindowHandle;
    Window->DeviceContext = DeviceContext;
    Window->OffscreenBitmap = OffscreenBitmap;
    DLLAppendTail(&Manager->Windows, Window);

    //
    // Post the window-created event.
    //

    os_event Event = {};
    Event.Type = OSEventType_WindowCreated;
    Event.Window = Window;
    Win32EnqueueEvent(Manager, &Event);

    return Window;
}

internal void
OSDestroyWindow(os_window_manager* Manager, os_window** WindowPtr)
{
    if (!Manager || !WindowPtr || !(*WindowPtr))
    {
        return;
    }

    os_window* Window = *WindowPtr;
    *WindowPtr = NULL;

    // @Incomplete: We should ensure that there are no queued events that target
    // this window before destroying it!    -- avrtraian 22 May 2026

    ReleaseDC(Window->Handle, Window->DeviceContext);
    DestroyWindow(Window->Handle);
    OSVirtualFree(Window->OffscreenBitmap.Data, 0, OSFreeType_Release);

    DLLRemove(&Manager->Windows, Window);
    ZERO_STRUCT(Window);
    StackPush(&Manager->WindowFreelist, Window);
}

internal bitmap*
OSGetOffscreenBitmap(os_window_manager* Manager, os_window* Window)
{
    if (!Manager || !Window)
    {
        return NULL;
    }

    return &Window->OffscreenBitmap;
}

internal void
OSPresentOffscreenBitmap(os_window_manager* Manager, os_window* Window)
{
    if (!Manager || !Window)
    {
        return;
    }

    v2u WindowSize = Win32GetWindowSize(Window->Handle);
    if (WindowSize.X == 0 || WindowSize.Y == 0)
    {
        return;
    }

    bitmap* Bitmap = &Window->OffscreenBitmap;
    if (Bitmap->Width == 0 || Bitmap->Height == 0)
    {
        return;
    }
    
    BITMAPINFO BitmapInfo = {};
    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = Bitmap->Width;
    BitmapInfo.bmiHeader.biHeight = Bitmap->Height;
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;

    StretchDIBits(Window->DeviceContext,
                  0, 0, WindowSize.X, WindowSize.Y,
                  0, 0, Bitmap->Width, Bitmap->Height,
                  Bitmap->Data, &BitmapInfo, DIB_RGB_COLORS, SRCCOPY);
}

internal b8
Win32DequeueEvent(os_window_manager* Manager, os_event* OutEvent)
{
    win32_event* Event = NULL;
    QueueDequeue(&Manager->EventQueue, Event);
    if (Event == NULL)
    {
        // No events in the queue.
        return false;
    }

    COPY_STRUCT(OutEvent, &Event->Event);
    StackPush(&Manager->EventFreelist, Event);
    return true;
}

internal void
OSWaitForNextEvent(os_window_manager* Manager, os_event* OutEvent)
{
    if (OutEvent == NULL)
    {
        return;
    }
    ZERO_STRUCT(OutEvent);

    if (Manager == NULL)
    {
        // If the 'OSInitWindowManager' has failed for whatever reason and the user
        // has not checked its return value, we should not enter an infinite loop.
        // This will ensure that that as long as the calling code handles the exit
        // events, the application will close immediately.  -- avrtraian 22 May 2026
        OutEvent->Type = OSEventType_ApplicationClosed;        
        return;
    }

    if (Manager->Windows.Head == NULL)
    {
        // There are no active windows. Post an application-closed event.
        OutEvent->Type = OSEventType_ApplicationClosed;        
        return;
    }

    while (true)
    {
        MSG Message = {};
        if (GetMessageA(&Message, NULL, 0, 0) > 0)
        {
            TranslateMessage(&Message);
            DispatchMessageA(&Message);
        }

        if (Win32DequeueEvent(Manager, OutEvent))
        {
            return;
        }
    }
}

internal b8
OSGetNextEvent(os_window_manager* Manager, os_event* OutEvent)
{
    if (OutEvent == NULL)
    {
        return false;
    }
    ZERO_STRUCT(OutEvent);    

    if (Manager == NULL)
    {
        // If the 'OSInitWindowManager' has failed for whatever reason and the user
        // has not checked its return value, we should not enter an infinite loop.
        // This will ensure that that as long as the calling code handles the exit
        // events, the application will close immediately.  -- avrtraian 22 May 2026
        OutEvent->Type = OSEventType_ApplicationClosed;        
        return true;
    }

    if (Manager->Windows.Head == NULL)
    {
        // There are no active windows. Post an application-closed event.
        OutEvent->Type = OSEventType_ApplicationClosed;        
        return true;
    }

    MSG Message = {};
    while (PeekMessageA(&Message, NULL, 0, 0, PM_REMOVE) > 0)
    {
        TranslateMessage(&Message);
        DispatchMessageA(&Message);
    }

    b8 Result = Win32DequeueEvent(Manager, OutEvent);
    return Result;
}

#endif // PLATFORM_OS_WINDOWS
