/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#include <Core/Assertion.h>
#include <Core/Memory/MemoryOperations.h>
#include <Editor/Editor.h>
#include <Platform/FileSystem.h>
#include <Platform/Memory.h>

//===============================================================================================
// EDITOR INPUT BUFFER.
//===============================================================================================

bool input_buffer_ensure_capacity(InputBuffer* input_buffer, usize buffer_capacity)
{
    if (input_buffer->text_buffer.count < buffer_capacity) {
        ReadWriteBytes new_buffer = platform_memory_allocate(buffer_capacity);
        copy_memory(new_buffer, input_buffer->text_buffer.bytes, input_buffer->text_buffer_used_byte_count);
        platform_memory_release(input_buffer->text_buffer.bytes, input_buffer->text_buffer.count);
        input_buffer->text_buffer.bytes = new_buffer;
        input_buffer->text_buffer.count = buffer_capacity;
        return true;
    }

    return false;
}

void input_buffer_insert_characters(InputBuffer* input_buffer, usize insert_byte_offset, ReadonlyByteSpan characters_to_insert_buffer)
{
    VERIFY(insert_byte_offset <= input_buffer->text_buffer_used_byte_count);

    if (input_buffer->text_buffer.count < input_buffer->text_buffer_used_byte_count + characters_to_insert_buffer.count) {
        ReadWriteBytes new_buffer = platform_memory_allocate(input_buffer->text_buffer_used_byte_count + characters_to_insert_buffer.count);
        copy_memory(new_buffer, input_buffer->text_buffer.bytes, insert_byte_offset);
        copy_memory(new_buffer + insert_byte_offset, characters_to_insert_buffer.bytes, characters_to_insert_buffer.count);
        copy_memory(
            new_buffer + insert_byte_offset + characters_to_insert_buffer.count,
            input_buffer->text_buffer.bytes + insert_byte_offset,
            input_buffer->text_buffer_used_byte_count - insert_byte_offset
        );
    }
    else {
        copy_memory_reversed(
            input_buffer->text_buffer.bytes + insert_byte_offset + characters_to_insert_buffer.count,
            input_buffer->text_buffer.bytes + insert_byte_offset,
            input_buffer->text_buffer_used_byte_count - insert_byte_offset
        );
        copy_memory(input_buffer->text_buffer.bytes + insert_byte_offset, characters_to_insert_buffer.bytes, characters_to_insert_buffer.count);
    }

    input_buffer->text_buffer_used_byte_count += characters_to_insert_buffer.count;
}

void input_buffer_remove_characters(InputBuffer* input_buffer, usize remove_byte_offset, usize number_of_bytes_to_remove)
{
    VERIFY(remove_byte_offset + number_of_bytes_to_remove <= input_buffer->text_buffer_used_byte_count);

    copy_memory(
        input_buffer->text_buffer.bytes + remove_byte_offset,
        input_buffer->text_buffer.bytes + remove_byte_offset + number_of_bytes_to_remove,
        input_buffer->text_buffer_used_byte_count - remove_byte_offset - number_of_bytes_to_remove
    );
    zero_memory(input_buffer->text_buffer.bytes + input_buffer->text_buffer_used_byte_count - number_of_bytes_to_remove, number_of_bytes_to_remove);
    input_buffer->text_buffer_used_byte_count -= number_of_bytes_to_remove;
}

//===============================================================================================
// EDITOR FONTS.
//===============================================================================================

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

//===============================================================================================
// EDITOR STATE.
//===============================================================================================

InputBuffer* editor_get_focused_input_buffer(EditorState* state)
{
    if (state->focused_input_panel == MAX_UINT32)
        return nullptr;

    return &state->input_panels[state->focused_input_panel].input_buffer;
}

//===============================================================================================
// EDITOR EVENT LOOP & LIFECYCLE.
//===============================================================================================

EditorState* editor_initialize(LinearArena* permanent_arena, Window window)
{
    EditorState* state = (EditorState*)core_linear_arena_allocate(permanent_arena, sizeof(EditorState));

    state->graphics_context = graphics_context_create(permanent_arena, GRAPHICS_CONTEXT_TYPE_CPU, window);
    state->swapchain_bitmap = graphics_context_allocate_bitmap(state->graphics_context, permanent_arena, 0, 0, GRAPHICS_BITMAP_USAGE_SWAPCHAIN);
    state->panel_bitmap = graphics_context_allocate_bitmap(state->graphics_context, permanent_arena, 1000, 1000, GRAPHICS_BITMAP_USAGE_RENDER_TARGET);

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
    graphics_context_release_bitmap(state->graphics_context, &state->panel_bitmap);
    graphics_context_release_bitmap(state->graphics_context, &state->swapchain_bitmap);
    graphics_context_destroy(&state->graphics_context);

    *state = {};
}

void editor_on_update(EditorState* state)
{
    DrawInstructionClear clear;
    clear.instruction_code = DRAW_INSTRUCTION_CODE_CLEAR;
    clear.target_bitmap = state->panel_bitmap;
    clear.clear_color = linear_color(255, 0, 0);
    draw_list_encode_instruction_clear(&state->draw_list, &clear);

    DrawInstructionPaintQuad paint_quad;
    paint_quad.instruction_code = DRAW_INSTRUCTION_CODE_PAINT_QUAD;
    paint_quad.target_bitmap = state->panel_bitmap;
    paint_quad.quad_surface = rect(100, 100, 500, 500);
    paint_quad.quad_color = linear_color(0, 0, 255);
    draw_list_encode_instruction_paint_quad(&state->draw_list, &paint_quad);

    DrawInstructionPaintGlyph paint_glyph;
    paint_glyph.instruction_code = DRAW_INSTRUCTION_CODE_PAINT_GLYPH;
    paint_glyph.target_bitmap = state->panel_bitmap;
    paint_glyph.glyph_bitmap = font_get_glyph(editor_get_font_from_id(state, EDITOR_FONT_ID_TEXT), 'A')->bitmap;
    paint_glyph.glyph_offset = vec2i(700, 200);
    paint_glyph.glyph_color = linear_color(0, 255, 0);
    draw_list_encode_instruction_paint_glyph(&state->draw_list, &paint_glyph);

    DrawInstructionCopyBitmap copy_bitmap;
    copy_bitmap.instruction_code = DRAW_INSTRUCTION_CODE_COPY_BITMAP;
    copy_bitmap.destination_bitmap = state->swapchain_bitmap;
    copy_bitmap.source_bitmap = state->panel_bitmap;
    copy_bitmap.copy_offset = vec2i(100, 100);
    draw_list_encode_instruction_copy_bitmap(&state->draw_list, &copy_bitmap);

    draw_list_execute_sync(&state->draw_list, state->graphics_context);
    draw_list_reset(&state->draw_list);

    graphics_context_swap_buffers(state->graphics_context);
}
