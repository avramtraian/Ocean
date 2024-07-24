/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#include <Core/Assertion.h>
#include <Core/Memory/MemoryOperations.h>
#include <Graphics/DrawList.h>
#include <Graphics/Software/SoftwareGraphicsContext.h>

void draw_list_initialize(DrawList* draw_list, ReadWriteByteSpan bytecode_buffer)
{
    draw_list->bytecode_buffer = bytecode_buffer;
    draw_list->bytecode_offset = 0;
    draw_list->bytecode_instruction_count = 0;
}

void draw_list_destroy(DrawList* draw_list)
{
    draw_list->bytecode_buffer = {};
    draw_list->bytecode_offset = 0;
    draw_list->bytecode_instruction_count = 0;
}

static usize get_draw_instruction_size(DrawInstructionCode instruction_code)
{
    switch (instruction_code) {
        case DRAW_INSTRUCTION_CODE_CLEAR: return sizeof(DrawInstructionClear);
        case DRAW_INSTRUCTION_CODE_PAINT_QUAD: return sizeof(DrawInstructionPaintQuad);
        case DRAW_INSTRUCTION_CODE_PAINT_GLYPH: return sizeof(DrawInstructionPaintGlyph);
    }

    VERIFY_NOT_REACHED;
    return 0;
}

//========================================================
// BEGIN --- DRAW INSTRUCTIONS EXECUTION IMPLEMENTATIONS.
//========================================================

static void execute_draw_instruction_clear(GraphicsContext, const DrawInstructionClear* instruction)
{
    const SoftwareGraphicsBitmap* target_bitmap = (const SoftwareGraphicsBitmap*)instruction->target_bitmap;

    switch (target_bitmap->usage) {
        case GRAPHICS_BITMAP_USAGE_RENDER_TARGET:
        case GRAPHICS_BITMAP_USAGE_SWAPCHAIN: {
            const usize pixel_count = (usize)target_bitmap->width * target_bitmap->height;
            const u32 packed_clear_color = color_pack_linear_to_u32_bgra(instruction->clear_color);

            u32* pixel_iterator = (u32*)target_bitmap->data;
            u32* pixel_iterator_end = pixel_iterator + pixel_count;

            while (pixel_iterator != pixel_iterator_end) {
                *pixel_iterator = packed_clear_color;
                ++pixel_iterator;
            }

            break;
        }

        case GRAPHICS_BITMAP_USAGE_FONT: {
            set_memory(target_bitmap->data, instruction->clear_color.red, target_bitmap->pitch * (usize)target_bitmap->height);
            break;
        }

        default: {
            VERIFY_NOT_REACHED;
            return;
        }
    }
}

static void execute_draw_instruction_paint_quad(GraphicsContext, const DrawInstructionPaintQuad* instruction)
{
    const SoftwareGraphicsBitmap* target_bitmap = (const SoftwareGraphicsBitmap*)instruction->target_bitmap;
    const Rect quad = instruction->quad_surface;
    const u32 quad_packed_color = color_pack_linear_to_u32_bgra(instruction->quad_color);

    VERIFY(0 <= quad.offset.x && quad.offset.x + quad.extent.x <= target_bitmap->width);
    VERIFY(0 <= quad.offset.y && quad.offset.y + quad.extent.y <= target_bitmap->height);

    switch (target_bitmap->usage) {
        case GRAPHICS_BITMAP_USAGE_RENDER_TARGET:
        case GRAPHICS_BITMAP_USAGE_SWAPCHAIN: {
            u32* pixel_iterator = (u32*)target_bitmap->data + (quad.offset.x + quad.offset.y * target_bitmap->width);
            u32* pixel_iterator_end_of_row = pixel_iterator + quad.extent.x;

            const usize pixel_iterator_row_jump_offset = target_bitmap->width - quad.extent.x;

            for (u32 offset_y = 0; offset_y < quad.extent.y; ++offset_y) {
                while (pixel_iterator != pixel_iterator_end_of_row) {
                    *pixel_iterator = quad_packed_color;
                    ++pixel_iterator;
                }

                pixel_iterator += pixel_iterator_row_jump_offset;
                pixel_iterator_end_of_row += target_bitmap->width;
            }

            break;
        }

        case GRAPHICS_BITMAP_USAGE_FONT: {
            // NOTE: We can't paint quads to a bitmap that is used for font rendering.
            //       This is considered an invalid instruction, as it should never be issued.

            // TODO: Inform the caller about this error instead of crashing.
            VERIFY_NOT_REACHED;
            return;
        }

        default: {
            VERIFY_NOT_REACHED;
            return;
        }
    }
}

static void execute_draw_instruction_paint_glyph(GraphicsContext, const DrawInstructionPaintGlyph* instruction)
{
    const SoftwareGraphicsBitmap* target_bitmap = (const SoftwareGraphicsBitmap*)instruction->target_bitmap;
    const SoftwareGraphicsBitmap* glyph_bitmap = (const SoftwareGraphicsBitmap*)instruction->glyph_bitmap;

    VERIFY(target_bitmap->usage == GRAPHICS_BITMAP_USAGE_RENDER_TARGET || target_bitmap->usage == GRAPHICS_BITMAP_USAGE_SWAPCHAIN);
    VERIFY(glyph_bitmap->usage == GRAPHICS_BITMAP_USAGE_FONT);

    const Vector2i offset = instruction->glyph_offset;
    VERIFY(0 <= offset.x && offset.x + glyph_bitmap->width <= target_bitmap->width);
    VERIFY(0 <= offset.y && offset.y + glyph_bitmap->height <= target_bitmap->height);

    u32* dst_pixel_iterator = (u32*)target_bitmap->data + ((usize)offset.x + (usize)offset.y * (usize)target_bitmap->width);
    const u8* src_pixel_iterator = glyph_bitmap->data;
    const usize dst_pixel_iterator_row_jump_count = target_bitmap->width - glyph_bitmap->width;

    for (u32 y_offset = 0; y_offset < glyph_bitmap->height; ++y_offset) {
        for (u32 x_offset = 0; x_offset < glyph_bitmap->width; ++x_offset) {
            const float glyph_alpha = *src_pixel_iterator++ / 255.0F;
            const LinearColor dst_color =
                color_blend_linear_colors(color_unpack_linear_from_u32_bgra(*dst_pixel_iterator), instruction->glyph_color, glyph_alpha);
            *dst_pixel_iterator++ = color_pack_linear_to_u32_bgra(dst_color);
        }

        dst_pixel_iterator += dst_pixel_iterator_row_jump_count;
    }
}

//========================================================
// END --- DRAW INSTRUCTIONS EXECUTION IMPLEMENTATIONS.
//========================================================

static usize draw_list_dispatch_instruction(GraphicsContext graphics_context, ReadonlyBytes instruction_data, usize instruction_data_max_size)
{
    // NOTE: The first byte of any draw instruction represents the instruction code.
    const DrawInstructionCode instruction_code = instruction_data[0];
    const usize instruction_size = get_draw_instruction_size(instruction_code);
    if (instruction_size > instruction_data_max_size) {
        // TODO: Inform the caller about this error instead of silently handling it.
        return 0;
    }

    switch (instruction_code) {
        case DRAW_INSTRUCTION_CODE_CLEAR: {
            execute_draw_instruction_clear(graphics_context, (const DrawInstructionClear*)instruction_data);
            break;
        }

        case DRAW_INSTRUCTION_CODE_PAINT_QUAD: {
            execute_draw_instruction_paint_quad(graphics_context, (const DrawInstructionPaintQuad*)instruction_data);
            break;
        }

        case DRAW_INSTRUCTION_CODE_PAINT_GLYPH: {
            execute_draw_instruction_paint_glyph(graphics_context, (const DrawInstructionPaintGlyph*)instruction_data);
            break;
        }

        default: {
            VERIFY_NOT_REACHED;
            return 0;
        }
    }

    return instruction_size;
}

void draw_list_execute_sync(const DrawList* draw_list, GraphicsContext graphics_context)
{
    usize execution_offset = 0;
    while (execution_offset < draw_list->bytecode_offset) {
        const usize instruction_size = draw_list_dispatch_instruction(
            graphics_context, draw_list->bytecode_buffer.bytes + execution_offset, draw_list->bytecode_offset - execution_offset
        );
        execution_offset += instruction_size;
    }
}

void draw_list_reset(DrawList* draw_list)
{
    draw_list->bytecode_offset = 0;
    draw_list->bytecode_instruction_count = 0;
}

void draw_list_encode_instruction(DrawList* draw_list, DrawInstructionCode instruction_code, ReadonlyBytes instruction_data)
{
    // NOTE: The first byte of any draw instruction represents the instruction code.
    if (instruction_data[0] != instruction_code) {
        // TODO: Inform the caller about this error instead of silently handling it by ignoring the encode command.
        return;
    }

    const usize instruction_size = get_draw_instruction_size(instruction_code);
    VERIFY(draw_list->bytecode_offset + instruction_size <= draw_list->bytecode_buffer.count);

    copy_memory(draw_list->bytecode_buffer.bytes + draw_list->bytecode_offset, instruction_data, instruction_size);
    draw_list->bytecode_offset += instruction_size;
    draw_list->bytecode_instruction_count++;
}
