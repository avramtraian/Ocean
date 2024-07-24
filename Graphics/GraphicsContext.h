/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#pragma once

#include <Core/Memory/Arena.h>
#include <Core/Memory/MemorySpan.h>
#include <Core/Types.h>
#include <Platform/Window.h>

// NOTE: As the graphics context can have multiple implementations, the end-user is provided with an opaque handle.
//       The graphics context implementation will expand the handle to the proper implementation object.
typedef void* GraphicsContext;

enum GraphicsContextTypeEnum {
    GRAPHICS_CONTEXT_TYPE_NONE = 0,
    GRAPHICS_CONTEXT_TYPE_CPU,
    GRAPHICS_CONTEXT_TYPE_GPU,
};
typedef u8 GraphicsContextType;

GraphicsContext graphics_context_create(LinearArena* arena, GraphicsContextType graphics_context_type, Window window);
void graphics_context_destroy(GraphicsContext* graphics_context);

void graphics_context_swap_buffers(GraphicsContext graphics_context);

// NOTE: Opaque handle towards a bitmap.
typedef void* GraphicsBitmap;

enum GraphicsBitmapUsageEnum {
    GRAPHICS_BITMAP_USAGE_NONE = 0,
    GRAPHICS_BITMAP_USAGE_RENDER_TARGET,
    GRAPHICS_BITMAP_USAGE_SWAPCHAIN,
    GRAPHICS_BITMAP_USAGE_FONT,
};
typedef u8 GraphicsBitmapUsage;

// NOTE: If the bitmap usage is set to 'GRAPHICS_BITMAP_USAGE_SWAPCHAIN' the provided width and height parameters
//       will be ignored and instead the window graphics context's window dimensions will be used.
// NOTE: Depending on the usage of the bitmap, the pixel data might be allocated from the provided linear arena
//       or directly from the platform layer (heap).
GraphicsBitmap
graphics_context_allocate_bitmap(GraphicsContext graphics_context, LinearArena* arena, u32 width, u32 height, GraphicsBitmapUsage usage);
void graphics_context_release_bitmap(GraphicsContext graphics_context, GraphicsBitmap* graphics_bitmap);

// NOTE: The provided byte buffer is copied to the bitmap internal buffer. It is responsability of the
//       caller to manage the provided buffer after the function invocation.
void graphics_bitmap_set_data(GraphicsContext graphics_context, GraphicsBitmap graphics_bitmap, ReadonlyByteSpan bitmap_data);

u32 graphics_bitmap_get_width(GraphicsBitmap graphics_bitmap);
u32 graphics_bitmap_get_height(GraphicsBitmap graphics_bitmap);
