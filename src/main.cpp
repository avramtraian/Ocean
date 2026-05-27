// Copyright (c) 2026 Traian Avram. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause.

#include "core.h"
#include "graphics.h"
#include "math_utils.h"
#include "os.h"

#include "core.cpp"
#include "graphics.cpp"
#include "os_windows.cpp"

struct editor_state
{
    arena* PermanentArena;
    arena* FrameArena;
};

internal s32
OSEntryPoint(string8* Arguments, u32 ArgumentCount)
{
    arena PermanentArena = {};
    arena FrameArena = {};

    ArenaInitialize(&PermanentArena, MiB(4), KiB(64));
    ArenaInitialize(&FrameArena, GiB(1), MiB(4));

    editor_state EditorState = {};
    EditorState.PermanentArena = &PermanentArena;
    EditorState.FrameArena = &FrameArena;

    os_window_manager* WindowManager = OSInitWindowManager(EditorState.PermanentArena);
    os_window* Window = OSCreateWindow(WindowManager,
                                       Str8Lit("editor5 @ Development Build"),
                                       OSWindowMode_Maximized);

    while (true)
    {
        os_event Event = {};
        OSWaitForNextEvent(WindowManager, &Event);
        if (Event.Type == OSEventType_ApplicationClosed)
        {
            break;
        }
        if (Event.Type == OSEventType_WindowClosed)
        {
            ASSERT(Event->Window == Window);
            // Destroying the window causes the window manager to post an application-closed
            // event, which in turn causes the event loop to terminate.
            OSDestroyWindow(WindowManager, &Window);
            continue;
        }

        bitmap* OffscreenBitmap = OSGetOffscreenBitmap(WindowManager, Window);
        // ...
        OSPresentOffscreenBitmap(WindowManager, Window);
    }

    OSDestroyWindow(WindowManager, &Window);
    return 0;
}
