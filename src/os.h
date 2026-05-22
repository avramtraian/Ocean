// Copyright (c) 2026 Traian Avram. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause.

#ifndef OS_H
#define OS_H

#include "core.h"
#include "graphics.h"
#include "math.h"

//==============================================================================
// GENERAL-PURPOSE OS LAYER.
//==============================================================================

//
// The platform-agnostic entry point that is invoked by the platform layer.
// This function is not implemented by the platform layer, but rather invoked
// by the native entry point (which is automatically defined in the platform
// layer).                                  -- avrtraian 22 May 2026
//
internal s32 OSEntryPoint(string8* Arguments, u32 ArgumentCount);

typedef u8 os_allocate_type_bits;
enum os_allocate_type_enum : os_allocate_type_bits
{
    OSAllocateType_Reserve = 1 << 0,
    OSAllocateType_Commit  = 1 << 1,
};

enum os_free_type : u8
{
    OSFreeType_Decommit,
    OSFreeType_Release,
};

internal usize OSGetMemoryPageSize();

internal usize OSGetMemoryPageAligned(usize UnalignedSize);

internal void* OSVirtualAllocate(void* Address,
                                 usize Size,
                                 os_allocate_type_bits AllocateType);

internal b8 OSVirtualFree(void* Address, usize Size, os_free_type FreeType);

//==============================================================================
// GRAPHICAL OS LAYER.
//==============================================================================

struct os_window;
struct os_window_manager;

enum os_window_mode : u8
{
    OSWindowMode_Windowed,
    OSWindowMode_Fullscreen,
    OSWindowMode_Maximized,
    OSWindowMode_Minimized,
};

enum os_event_type : u8
{
    OSEventType_ApplicationClosed,
    OSEventType_WindowResized,
    OSEventType_WindowCreated,
    OSEventType_WindowClosed,

    OSEventType_KeyPressed,
    OSEventType_KeyReleased,
    OSEventType_KeyInput,

    OSEventType_MouseMoved,
    OSEventType_MouseButtonPressed,
    OSEventType_MouseButtonReleased,
    OSEventType_MouseWheelScrolled,
};

struct os_event_window_resized
{
    u32            Width;
    u32            Height;
    os_window_mode Mode;
};

enum os_key_code : u16
{
    OSKeyCode_Invalid = 0,
    OSKeyCode_A,
    OSKeyCode_B,
    OSKeyCode_C,
    OSKeyCode_D,
    OSKeyCode_MaxEnumCount,
};

typedef u8 os_key_modifier_bits;
enum os_key_modifier_enum : os_key_modifier_bits
{
    OSKeyModifier_None    = 0,
    OSKeyModifier_Control = 1 << 0,
    OSKeyModifier_Shift   = 1 << 1,
    OSKeyModifier_Alt     = 1 << 2,
};

struct os_event_key_pressed
{
    os_key_code          KeyCode;
    os_key_modifier_bits Modifiers;
    u32                  RepeatCount;
};

struct os_event_key_released
{
    os_key_code KeyCode;
};

struct os_event_key_input
{
    u32 CodePoint; // Unicode.
    u32 RepeatCount;
};

enum os_mouse_button : u8
{
    OSMouseButton_Invalid = 0,
    OSMouseButton_Left,
    OSMouseButton_Right,
    OSMouseButton_Middle,
    OSMouseButton_MaxEnumCount,
};

// NOTE: The relative mouse position available in all mouse events is relative to
// the bottom-left corner of the source window's client region. Also note that we
// use the Y-up convention.                     -- avrtraian 22 May 2026

struct os_event_mouse_moved
{
    v2s                  Position;
    v2s                  AbsolutePosition;
    b8                   IsInsideClientRegion;
    os_key_modifier_bits Modifiers;
    b8                   ButtonIsDown[OSMouseButton_MaxEnumCount];
};

struct os_event_mouse_button_pressed
{
    os_mouse_button      Button;
    os_key_modifier_bits Modifiers;
    v2s                  Position; // Unlike the mouse-moved event, the moue is guaranteed to be inside the client region.
    v2s                  AbsolutePosition;
};

struct os_event_mouse_button_released
{
    os_mouse_button      Button;
    os_key_modifier_bits Modifiers;
    v2s                  Position; // Unlike the mouse-moved event, the moue is guaranteed to be inside the client region.
    v2s                  AbsolutePosition;
};

struct os_event_mouse_wheel_scrolled
{
    f32                  VerticalDelta;
    f32                  HorizontalDelta;
    os_key_modifier_bits Modifiers;
    v2s                  Position; // Unlike the mouse-moved event, the moue is guaranteed to be inside the client region.
    v2s                  AbsolutePosition;
};

struct os_event
{
    os_event_type Type;
    os_window*    Window;
    union
    {
        os_event_window_resized        WindowResized;
        os_event_key_pressed           KeyPressed;
        os_event_key_released          KeyReleased;
        os_event_key_input             KeyInput;
        os_event_mouse_moved           MouseMoved;
        os_event_mouse_button_pressed  MouseButtonPressed;
        os_event_mouse_button_released MouseButtonReleased;
        os_event_mouse_wheel_scrolled  MouseWheelScrolled;
    };
};

internal os_window_manager* OSInitWindowManager(arena* Arena);

internal os_window* OSCreateWindow(os_window_manager* Manager,
                                   string8 Title,
                                   os_window_mode Mode);

internal void OSDestroyWindow(os_window_manager* Manager, os_window** Window);

internal bitmap* OSGetOffscreenBitmap(os_window_manager* Manager, os_window* Window);

internal void OSPresentOffscreenBitmap(os_window_manager* Manager, os_window* Window);

// Blocks the calling thread until a new event is posted. The event written to the
// out parameter is always valid.
internal void OSWaitForNextEvent(os_window_manager* Manager, os_event* OutEvent);

// If there is no event posted in the internal queue this function returns immediately.
// You must always check the return value before accessing the value written in the out
// parameter.
internal b8 OSGetNextEvent(os_window_manager* Manager, os_event* OutEvent);

#endif // OS_H
