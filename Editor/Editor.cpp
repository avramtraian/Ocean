/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#include <Core/Assertion.h>
#include <Core/Memory/MemoryOperations.h>
#include <Editor/Editor.h>
#include <Platform/FileSystem.h>

static void editor_initialize_font_table(EditorState* state, LinearArena* arena)
{
    state->font_table.font_count = 1;
    state->font_table.fonts = (Font*)core_linear_arena_allocate(arena, state->font_table.font_count * sizeof(Font));

    // TODO: The fact that the TTF file buffer is allocated by the linear arena is not ideal, as it will stay
    //       in memory until the end of the program lifetime. We only use it here, to load the fonts, after
    //       which we can discard it.
    ReadWriteByteSpan ttf_file_buffer;
    ttf_file_buffer.count = filesystem_get_file_size("../../consola.ttf");
    VERIFY(ttf_file_buffer.count != INVALID_SIZE);
    ttf_file_buffer.bytes = (ReadWriteBytes)core_linear_arena_allocate(arena, ttf_file_buffer.count);
    filesystem_read_entire_file("../../consola.ttf", ttf_file_buffer);

    const u32 default_font_index = 0;
    Font* default_font = state->font_table.fonts + default_font_index;
    font_create_from_memory(default_font, arena, 150.0F, readonly_byte_span(ttf_file_buffer), state->graphics_context);

    state->font_table.font_indices[EDITOR_FONT_ID_NONE] = default_font_index;
    state->font_table.font_indices[EDITOR_FONT_ID_TEXT] = default_font_index;
    state->font_table.font_indices[EDITOR_FONT_ID_UI_TEXT] = default_font_index;
}

static void editor_destroy_font_table(EditorState* state)
{
    for (u32 font_index = 0; font_index < state->font_table.font_count; ++font_index) {
        Font* font = state->font_table.fonts + font_index;
        font_destroy(font, state->graphics_context);
    }

    zero_memory(state->font_table.font_indices, sizeof(state->font_table.font_indices));
}

static const Font* editor_get_font_from_id(const EditorState* state, EditorFontID font_id)
{
    VERIFY(font_id < EDITOR_FONT_ID_UI_COUNT);
    VERIFY(font_id != EDITOR_FONT_ID_NONE);

    const u32 font_index = state->font_table.font_indices[font_id];
    return state->font_table.fonts + font_index;
}

EditorState* editor_initialize(LinearArena* permanent_arena, Window window)
{
    EditorState* state = (EditorState*)core_linear_arena_allocate(permanent_arena, sizeof(EditorState));

    state->graphics_context = graphics_context_create(permanent_arena, GRAPHICS_CONTEXT_TYPE_CPU, window);
    state->swapchain_bitmap = graphics_context_allocate_bitmap(state->graphics_context, permanent_arena, 0, 0, GRAPHICS_BITMAP_USAGE_SWAPCHAIN);

    ReadWriteByteSpan draw_list_bytecode_buffer;
    draw_list_bytecode_buffer.count = 4 * MiB;
    draw_list_bytecode_buffer.bytes = (ReadWriteBytes)core_linear_arena_allocate(permanent_arena, draw_list_bytecode_buffer.count);
    draw_list_initialize(&state->draw_list, draw_list_bytecode_buffer);

    editor_initialize_font_table(state, permanent_arena);

    return state;
}

void editor_shutdown(EditorState* state)
{
    editor_destroy_font_table(state);

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
