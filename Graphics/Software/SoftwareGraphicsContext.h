/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#pragma once

#include <Graphics/GraphicsContext.h>
#include <Platform/Window.h>

struct SoftwareGraphicsContext {
    Window window;
    GraphicsBitmap swapchain_bitmap;
};

struct SoftwareGraphicsBitmap {
    u32 width;
    u32 height;
    GraphicsBitmapUsage usage;
    usize pitch;
    ReadWriteBytes data;
};
