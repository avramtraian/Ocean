/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#pragma once

#include <Core/Math/Color.h>
#include <Core/Math/Rect.h>
#include <Core/Memory/MemorySpan.h>
#include <Graphics/GraphicsContext.h>

enum DrawInstructionCodeEnum {
    DRAW_INSTRUCTION_CODE_NONE = 0,
    DRAW_INSTRUCTION_CODE_CLEAR,
    DRAW_INSTRUCTION_CODE_PAINT_QUAD,
};
typedef u8 DrawInstructionCode;

struct DrawInstructionClear {
    // NOTE: Should always be 'DRAW_INSTRUCTION_CODE_CLEAR'.
    DrawInstructionCode instruction_code;
    GraphicsBitmap target_bitmap;
    LinearColor clear_color;
};

struct DrawInstructionPaintQuad {
    // NOTE: Should always be 'DRAW_INSTRUCTION_CODE_PAINT_QUAD'.
    DrawInstructionCode instruction_code;
    GraphicsBitmap target_bitmap;
    Rect quad_surface;
    LinearColor quad_color;
};

struct DrawList {
    ReadWriteByteSpan bytecode_buffer;
    usize bytecode_offset;
    usize bytecode_instruction_count;
};

void draw_list_initialize(DrawList* draw_list, ReadWriteByteSpan bytecode_buffer);
void draw_list_destroy(DrawList* draw_list);

void draw_list_reset(DrawList* draw_list);

void draw_list_execute_sync(const DrawList* draw_list, GraphicsContext graphics_context);

void draw_list_encode_instruction(DrawList* draw_list, DrawInstructionCode instruction_code, ReadonlyBytes instruction_data);

#define DEFINE_DRAW_LIST_ENCODE_INSTRUCTION(instruction_code, instruction_struct, instruction_name)                         \
    inline void draw_list_encode_instruction_##instruction_name(DrawList* draw_list, const instruction_struct* instruction) \
    {                                                                                                                       \
        draw_list_encode_instruction(draw_list, instruction_code, (ReadonlyBytes)instruction);                              \
    }

DEFINE_DRAW_LIST_ENCODE_INSTRUCTION(DRAW_INSTRUCTION_CODE_CLEAR, DrawInstructionClear, clear);
DEFINE_DRAW_LIST_ENCODE_INSTRUCTION(DRAW_INSTRUCTION_CODE_PAINT_QUAD, DrawInstructionPaintQuad, paint_quad);

#undef DEFINE_DRAW_LIST_ENCODE_INSTRUCTION
