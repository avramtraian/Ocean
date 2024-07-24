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
    }

    VERIFY_NOT_REACHED;
    return 0;
}

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
            execute_draw_instruction_clear(graphics_context, (DrawInstructionClear*)instruction_data);
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
