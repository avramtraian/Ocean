/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#include <Core/Assertion.h>
#include <Graphics/GraphicsContext.h>
#include <Platform/Memory.h>
#include <Platform/SoftwareSwapchain.h>

struct SoftwareGraphicsContext {
    Window window;
    GraphicsBitmap swapchain_bitmap;
};

GraphicsContext graphics_context_create(LinearArena* arena, GraphicsContextType graphics_context_type, Window window)
{
    VERIFY(graphics_context_type == GRAPHICS_CONTEXT_TYPE_CPU);

    if (graphics_context_type == GRAPHICS_CONTEXT_TYPE_CPU) {
        SoftwareGraphicsContext* graphics_context = (SoftwareGraphicsContext*)core_linear_arena_allocate(arena, sizeof(SoftwareGraphicsContext));
        graphics_context->window = window;
        graphics_context->swapchain_bitmap = INVALID_HANDLE;
        return graphics_context;
    }

    VERIFY_NOT_REACHED;
    return INVALID_HANDLE;
}

void graphics_context_destroy(GraphicsContext* graphics_context)
{
    SoftwareGraphicsContext* context = (SoftwareGraphicsContext*)graphics_context;
    *context = {};
    *graphics_context = INVALID_HANDLE;
}

struct SoftwareGraphicsBitmap {
    u32 width;
    u32 height;
    GraphicsBitmapUsage usage;
    usize pitch;
    ReadWriteBytes data;
};

void graphics_context_swap_buffers(GraphicsContext graphics_context)
{
    const SoftwareGraphicsContext* context = (const SoftwareGraphicsContext*)graphics_context;
    const SoftwareGraphicsBitmap* swapchain_bitmap = (const SoftwareGraphicsBitmap*)context->swapchain_bitmap;
    VERIFY(swapchain_bitmap != INVALID_HANDLE);

    platform_software_swapchain_swap(context->window, swapchain_bitmap->width, swapchain_bitmap->height, swapchain_bitmap->data);
}

GraphicsBitmap
graphics_context_allocate_bitmap(GraphicsContext graphics_context, LinearArena* arena, u32 width, u32 height, GraphicsBitmapUsage usage)
{
    SoftwareGraphicsBitmap* bitmap = (SoftwareGraphicsBitmap*)core_linear_arena_allocate(arena, sizeof(SoftwareGraphicsBitmap));
    bitmap->width = width;
    bitmap->height = height;
    bitmap->usage = usage;

    if (bitmap->usage == GRAPHICS_BITMAP_USAGE_SWAPCHAIN) {
        const SoftwareGraphicsContext* context = (const SoftwareGraphicsContext*)graphics_context;
        bitmap->width = platform_window_width(context->window);
        bitmap->height = platform_window_height(context->window);
    }

    switch (bitmap->usage) {
        case GRAPHICS_BITMAP_USAGE_RENDER_TARGET:
        case GRAPHICS_BITMAP_USAGE_SWAPCHAIN: {
#define GRAPHICS_BITMAP_RENDER_TARGET_BYTES_PER_PIXEL (4)
            bitmap->pitch = (usize)bitmap->width * GRAPHICS_BITMAP_RENDER_TARGET_BYTES_PER_PIXEL;
#undef GRAPHICS_BITMAP_RENDER_TARGET_BYTES_PER_PIXEL
            bitmap->data = platform_memory_allocate(bitmap->pitch * (usize)bitmap->height);
            break;
        }

        case GRAPHICS_BITMAP_USAGE_FONT: {
#define GRAPHICS_BITMAP_FONT_BYTES_PER_PIXEL (1)
            bitmap->pitch = (usize)bitmap->width * GRAPHICS_BITMAP_FONT_BYTES_PER_PIXEL;
#undef GRAPHICS_BITMAP_FONT_BYTES_PER_PIXEL
            bitmap->data = (ReadWriteBytes)core_linear_arena_allocate(arena, bitmap->pitch * (usize)bitmap->height);
            break;
        }

        default: {
            VERIFY_NOT_REACHED;
            return INVALID_HANDLE;
        }
    }

    if (bitmap->usage == GRAPHICS_BITMAP_USAGE_SWAPCHAIN) {
        SoftwareGraphicsContext* context = (SoftwareGraphicsContext*)graphics_context;
        // NOTE: There can only be one instance of a swapchain bitmap at a time.
        VERIFY(context->swapchain_bitmap == INVALID_HANDLE);
        context->swapchain_bitmap = bitmap;
    }

    return bitmap;
}

void graphics_context_release_bitmap(GraphicsContext graphics_context, GraphicsBitmap* graphics_bitmap)
{
    SoftwareGraphicsBitmap* bitmap = (SoftwareGraphicsBitmap*)*graphics_bitmap;

    switch (bitmap->usage) {
        case GRAPHICS_BITMAP_USAGE_RENDER_TARGET:
        case GRAPHICS_BITMAP_USAGE_SWAPCHAIN: {
            platform_memory_release(bitmap->data, bitmap->pitch * (usize)bitmap->height);
            break;
        }

        case GRAPHICS_BITMAP_USAGE_FONT: {
            // NOTE: The bitmaps used for font rendering are allocated from a linear memory arena, thus
            //       they can't be individually released. This is not a memory leak.
            break;
        }

        default: {
            VERIFY_NOT_REACHED;
            return;
        }
    }

    if (bitmap->usage == GRAPHICS_BITMAP_USAGE_SWAPCHAIN) {
        SoftwareGraphicsContext* context = (SoftwareGraphicsContext*)graphics_context;
        VERIFY(context->swapchain_bitmap == bitmap);
        context->swapchain_bitmap = INVALID_HANDLE;
    }

    *bitmap = {};
    *graphics_bitmap = INVALID_HANDLE;
}

u32 graphics_bitmap_get_width(GraphicsBitmap graphics_bitmap)
{
    VERIFY(graphics_bitmap != INVALID_HANDLE);
    const SoftwareGraphicsBitmap* bitmap = (const SoftwareGraphicsBitmap*)graphics_bitmap;
    return bitmap->width;
}

u32 graphics_bitmap_get_height(GraphicsBitmap graphics_bitmap)
{
    VERIFY(graphics_bitmap != INVALID_HANDLE);
    const SoftwareGraphicsBitmap* bitmap = (const SoftwareGraphicsBitmap*)graphics_bitmap;
    return bitmap->height;
}
