/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#include <Editor/Editor.h>

EditorState* editor_initialize(LinearArena* permanent_arena, Window window)
{
    EditorState* state = (EditorState*)core_linear_arena_allocate(permanent_arena, sizeof(EditorState));

    state->graphics_context = graphics_context_create(permanent_arena, GRAPHICS_CONTEXT_TYPE_CPU, window);
    state->swapchain_bitmap = graphics_context_allocate_bitmap(state->graphics_context, permanent_arena, 0, 0, GRAPHICS_BITMAP_USAGE_SWAPCHAIN);

    ReadWriteByteSpan draw_list_bytecode_buffer;
    draw_list_bytecode_buffer.count = 4 * MiB;
    draw_list_bytecode_buffer.bytes = (ReadWriteBytes)core_linear_arena_allocate(permanent_arena, draw_list_bytecode_buffer.count);
    draw_list_initialize(&state->draw_list, draw_list_bytecode_buffer);

    return state;
}

void editor_shutdown(EditorState* state)
{
    draw_list_destroy(&state->draw_list);
    graphics_context_release_bitmap(state->graphics_context, &state->swapchain_bitmap);
    graphics_context_destroy(&state->graphics_context);

    *state = {};
}

void editor_on_update(EditorState* state)
{
    DrawInstructionClear clear;
    clear.instruction_code = DRAW_INSTRUCTION_CODE_CLEAR;
    clear.target_bitmap = state->swapchain_bitmap;
    clear.clear_color = linear_color(255, 0, 0);
    draw_list_encode_instruction_clear(&state->draw_list, &clear);

    DrawInstructionPaintQuad paint_quad;
    paint_quad.instruction_code = DRAW_INSTRUCTION_CODE_PAINT_QUAD;
    paint_quad.target_bitmap = state->swapchain_bitmap;
    paint_quad.quad_surface = rect(100, 100, 500, 500);
    paint_quad.quad_color = linear_color(0, 0, 255);
    draw_list_encode_instruction_paint_quad(&state->draw_list, &paint_quad);

    draw_list_execute_sync(&state->draw_list, state->graphics_context);
    draw_list_reset(&state->draw_list);

    graphics_context_swap_buffers(state->graphics_context);
}
