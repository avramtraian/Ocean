/**
 * Copyright (c) 2025 Traian Avram. All rights reserved.
 * This source file is part of the Ocean text editor.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#include "core.h"
#include "draw.h"
#include "editor.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE(traian): Widgets initialize implementations.
////////////////////////////////////////////////////////////////////////////////////////////////////

internal EDITOR_WIDGET_API_INITIALIZE(editor_widget_initialize)
{
    if (state == NULL || widget == NULL)
        return;

    for (u32 child_index = 0; child_index < widget->child_count; ++child_index) {
        EditorWidget *child_widget = (EditorWidget *)(widget->children[child_index]);
        if (child_widget && child_widget->initialize)
            child_widget->initialize(state, child_widget);
    }
}

internal EDITOR_WIDGET_API_INITIALIZE(panel_titlebar_widget_initialize)
{
    PanelTitlebarWidget *titlebar_widget = (PanelTitlebarWidget *)widget;

    // TODO: This value is absolutely arbitrary and should probably be exposed to
    // some kind of user configuration space. There is no assumption about this value.
    titlebar_widget->title_buffer_size = 128;
    titlebar_widget->title_buffer = (char *)memory_arena_allocate(state->memory->permanent_arena, titlebar_widget->title_buffer_size);

    editor_widget_initialize(state, widget);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE(traian): Widgets resize implementations.
////////////////////////////////////////////////////////////////////////////////////////////////////

internal EDITOR_WIDGET_API_RESIZE(editor_widget_resize)
{
    if (state == NULL || widget == NULL)
        return;

    for (u32 child_index = 0; child_index < widget->child_count; ++child_index) {
        EditorWidget *child_widget = (EditorWidget *)(widget->children[child_index]);
        if (child_widget && child_widget->resize)
            child_widget->resize(state, child_widget);
    }
}

internal EDITOR_WIDGET_API_RESIZE(panel_assembly_widget_resize)
{
    widget->surface_offset_x = 0;
    widget->surface_offset_y = 0;
    widget->surface_size_x = state->offscreen_bitmap->size_x;
    widget->surface_size_y = state->offscreen_bitmap->size_y;

    editor_widget_resize(state, widget);
}

internal EDITOR_WIDGET_API_RESIZE(panel_widget_resize)
{
    PanelWidget *panel_widget = (PanelWidget *)widget;
    PanelAssemblyWidget *panel_assembly_widget = (PanelAssemblyWidget *)widget->parent;

    const u32 panel_size_x = panel_assembly_widget->surface_size_x / panel_assembly_widget->panel_count;
    const u32 panel_size_y = panel_assembly_widget->surface_size_y;

    widget->surface_offset_x = panel_assembly_widget->surface_offset_x + (panel_widget->panel_index * panel_size_x);
    widget->surface_offset_y = panel_assembly_widget->surface_offset_y;
    widget->surface_size_x = panel_size_x;
    widget->surface_size_y = panel_size_y;

    // NOTE: To ensure that the panels perfectly fill the assembly widget, the last
    // panel is extended with the required amount.
    if (panel_widget->panel_index == panel_assembly_widget->panel_count - 1)
        widget->surface_size_x += (panel_assembly_widget->surface_size_x % panel_assembly_widget->panel_count);

    editor_widget_resize(state, widget);
}

internal EDITOR_WIDGET_API_RESIZE(panel_content_buffer_widget_resize)
{
    const u32 titlebar_height = 40; // TODO: Extract from settings.
    widget->surface_offset_x = widget->parent->surface_offset_x;
    widget->surface_offset_y = widget->parent->surface_offset_y + titlebar_height;
    widget->surface_size_x = widget->parent->surface_size_x;
    widget->surface_size_y = max_s32((s32)widget->parent->surface_size_y - (s32)titlebar_height, 0);

    editor_widget_resize(state, widget);
}

internal EDITOR_WIDGET_API_RESIZE(panel_titlebar_widget_resize)
{
    const u32 titlebar_height = 40; // TODO: Extract from settings.
    widget->surface_offset_x = widget->parent->surface_offset_x;
    widget->surface_offset_y = widget->parent->surface_offset_y;
    widget->surface_size_x = widget->parent->surface_size_x;
    widget->surface_size_y = min_u32(titlebar_height, widget->parent->surface_size_y);

    // TODO: Create a separate font ID for the titlebar text instead of using the default one.
    Font *font = state->fonts + FONT_ID_DEFAULT;

    const u32 cell_size_x = font->advance;
    const u32 cell_size_y = font->ascent + font->descent;

    u32 padding_x = 8; // TODO: Extract from settings.
    u32 viewport_size_x = max_s32((s32)widget->surface_size_x - (s32)(2 * padding_x), 0);

    const u32 viewport_size_y = min_u32(font->ascent + font->descent, widget->surface_size_y);
    const u32 padding_y = (widget->surface_size_y - viewport_size_y) / 2;

    u32 cell_count_x, cell_count_y;
    tiled_text_buffer_cell_count_from_viewport(
        viewport_size_x, viewport_size_y, cell_size_x, cell_size_y,
        0, false, &cell_count_x, &cell_count_y);
    cell_count_y = 1;

    PanelTitlebarWidget *titlebar_widget = (PanelTitlebarWidget *)widget;
    tiled_text_buffer_initialize(&titlebar_widget->text_buffer, state->memory->dynamic_resources_arena, cell_count_x, cell_count_y);
    tiled_text_buffer_set_cell_size(&titlebar_widget->text_buffer, cell_size_x, cell_size_y, 0);

    viewport_size_x = cell_count_x * cell_size_x;
    padding_x = (widget->surface_size_x - viewport_size_x) / 2;

    const u32 viewport_offset_x = widget->surface_offset_x + padding_x;
    const u32 viewport_offset_y = widget->surface_offset_y + padding_y;

    tiled_text_buffer_set_viewport(
        &titlebar_widget->text_buffer,
        viewport_offset_x, viewport_offset_y,
        viewport_size_x, viewport_size_y);

    editor_widget_resize(state, widget);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE(traian): Widgets update implementations.
////////////////////////////////////////////////////////////////////////////////////////////////////

internal EDITOR_WIDGET_API_UPDATE(editor_widget_update)
{
    if (state == NULL || widget == NULL)
        return;

    for (u32 child_index = 0; child_index < widget->child_count; ++child_index) {
        EditorWidget *child_widget = (EditorWidget *)(widget->children[child_index]);
        if (child_widget && child_widget->update)
            child_widget->update(state, child_widget);
    }
}

internal void
try_to_push_codepoint_to_tiled_text_buffer(TiledTextBuffer *buffer, u32 *cell_index_x, u32 cell_index_y, u32 codepoint, LinearColor color)
{
    if (*cell_index_x >= buffer->cell_count_x)
        return;

    TiledTextCell *cell = tiled_text_buffer_get_cell(buffer, (*cell_index_x)++, cell_index_y);
    cell->codepoint = codepoint;
    cell->color = color;
}

internal EDITOR_WIDGET_API_UPDATE(panel_titlebar_widget_update)
{
    PanelTitlebarWidget *titlebar_widget = (PanelTitlebarWidget *)widget;
    tiled_text_buffer_reset_cells(&titlebar_widget->text_buffer);
    ASSERT(titlebar_widget->text_buffer.cell_count_y == 1);

    // TODO: Used for testing purposes. Remove!
    {
        const char title[] = "Hello this is very COOL title!";
        copy_memory(titlebar_widget->title_buffer, title, sizeof(title));
        titlebar_widget->title.characters = titlebar_widget->title_buffer;
        titlebar_widget->title.byte_count = sizeof(title);
    }

    // NOTE: Space for each title subsection is allocated by the following table:
    // +------------------+-------------------+------------------------+------------------+------------------+--------------------------+
    // | title_cell_count | 3 cells for ' L#' | line_number_cell_count | 1 cell for space | 2 cells for 'C#' | column_number_cell_count |
    // +------------------+-------------------+------------------------+------------------+------------------+--------------------------+
    // The algorithm that calculates these values tries to prioritize, in the following order:
    //   1) The line number;
    //   2) The column number;
    //   3) The title;
    //   3) The padding.
    // If the title doesn't fit entirely, it will be postfixed with '...' (which are also included in the 'title_cell_count' value).

    const u32 total_cell_count = titlebar_widget->text_buffer.cell_count_x;
    const u32 unmutable_cell_count = 6; // 3 + 1 + 2
    const u32 line_number_cell_count = (u32)string_size_from_uint(titlebar_widget->line_number, NUMERIC_BASE_DECIMAL);
    const u32 column_number_cell_count = (u32)string_size_from_uint(titlebar_widget->column_number, NUMERIC_BASE_DECIMAL);
    const u32 title_cell_count = max_s32(total_cell_count - (unmutable_cell_count + line_number_cell_count + column_number_cell_count), 0);

    const LinearColor titlebar_text_color = linear_color(255, 255, 255); // TODO: Extract from settings.
    u32 cell_index = 0;
    
    // NOTE: Fill the buffer with the title contents.
    {
        usize title_byte_offset = 0;
        while (cell_index < title_cell_count && title_byte_offset < titlebar_widget->title.byte_count) {
            TiledTextCell *cell = tiled_text_buffer_get_cell(&titlebar_widget->text_buffer, cell_index++, 0);
            cell->codepoint = titlebar_widget->title.characters[title_byte_offset++];
            cell->color = titlebar_text_color;
        }
        cell_index = title_cell_count;
    }

    // TODO: Used for testing purposes. Remove!
    titlebar_widget->line_number = 12489;
    titlebar_widget->column_number = 54783;
    
    // NOTE: Fill the buffer with the line number contents.
    {
        if (title_cell_count > 0)
            try_to_push_codepoint_to_tiled_text_buffer(&titlebar_widget->text_buffer, &cell_index, 0, ' ', titlebar_text_color);

        try_to_push_codepoint_to_tiled_text_buffer(&titlebar_widget->text_buffer, &cell_index, 0, 'L', titlebar_text_color);
        try_to_push_codepoint_to_tiled_text_buffer(&titlebar_widget->text_buffer, &cell_index, 0, '#', titlebar_text_color);

        String line_number_string = string_from_uint(state->memory->work_arena, titlebar_widget->line_number, NUMERIC_BASE_DECIMAL);
        for (u32 byte_offset = 0; byte_offset < line_number_string.byte_count; ++byte_offset) {
            const u32 codepoint = line_number_string.characters[byte_offset];
            try_to_push_codepoint_to_tiled_text_buffer(&titlebar_widget->text_buffer, &cell_index, 0, codepoint, titlebar_text_color);
        }
    }

    // NOTE: Fill the buffer with the column number contents.
    {
        try_to_push_codepoint_to_tiled_text_buffer(&titlebar_widget->text_buffer, &cell_index, 0, ' ', titlebar_text_color);
        try_to_push_codepoint_to_tiled_text_buffer(&titlebar_widget->text_buffer, &cell_index, 0, 'C', titlebar_text_color);
        try_to_push_codepoint_to_tiled_text_buffer(&titlebar_widget->text_buffer, &cell_index, 0, '#', titlebar_text_color);

        String column_number_string = string_from_uint(state->memory->work_arena, titlebar_widget->column_number, NUMERIC_BASE_DECIMAL);
        for (u32 byte_offset = 0; byte_offset < column_number_string.byte_count; ++byte_offset) {
            const u32 codepoint = column_number_string.characters[byte_offset];
            try_to_push_codepoint_to_tiled_text_buffer(&titlebar_widget->text_buffer, &cell_index, 0, codepoint, titlebar_text_color);
        }
    }

    editor_widget_update(state, widget);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE(traian): Widgets paint implementations.
////////////////////////////////////////////////////////////////////////////////////////////////////

internal EDITOR_WIDGET_API_PAINT(editor_widget_paint)
{
    if (state == NULL || widget == NULL)
        return;

    for (u32 child_index = 0; child_index < widget->child_count; ++child_index) {
        EditorWidget *child_widget = (EditorWidget *)(widget->children[child_index]);
        if (child_widget && child_widget->paint)
            child_widget->paint(state, child_widget);
    }
}

internal EDITOR_WIDGET_API_PAINT(panel_content_buffer_widget_paint)
{
    draw_quad(
        state->offscreen_bitmap,
        widget->surface_offset_x, widget->surface_offset_y, widget->surface_size_x, widget->surface_size_y,
        linear_color(255, 0, 0));
}

internal EDITOR_WIDGET_API_PAINT(panel_titlebar_widget_paint)
{
    draw_quad(
        state->offscreen_bitmap,
        widget->surface_offset_x, widget->surface_offset_y, widget->surface_size_x, widget->surface_size_y,
        linear_color(0, 255, 0));

    PanelTitlebarWidget *titlebar_widget = (PanelTitlebarWidget *)widget;
    draw_tiled_text_buffer(state->offscreen_bitmap, &titlebar_widget->text_buffer, state->fonts + FONT_ID_DEFAULT);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE(traian): Widget construction.
////////////////////////////////////////////////////////////////////////////////////////////////////

internal EDITOR_WIDGET_API_CONSTRUCT(editor_widget_construct)
{
    widget->initialize = editor_widget_initialize;
    widget->resize = editor_widget_resize;
    widget->update = editor_widget_update;
    widget->paint = editor_widget_paint;
}

internal EDITOR_WIDGET_API_CONSTRUCT(panel_assembly_widget_construct)
{
    editor_widget_construct(widget);
    widget->resize = panel_assembly_widget_resize;
}

internal EDITOR_WIDGET_API_CONSTRUCT(panel_widget_construct)
{
    editor_widget_construct(widget);
    widget->resize = panel_widget_resize;
}

internal EDITOR_WIDGET_API_CONSTRUCT(panel_content_buffer_widget_construct)
{
    editor_widget_construct(widget);
    widget->resize = panel_content_buffer_widget_resize;
    widget->paint = panel_content_buffer_widget_paint;
}

internal EDITOR_WIDGET_API_CONSTRUCT(panel_titlebar_widget_construct)
{
    editor_widget_construct(widget);
    widget->initialize = panel_titlebar_widget_initialize;
    widget->resize = panel_titlebar_widget_resize;
    widget->update = panel_titlebar_widget_update;
    widget->paint = panel_titlebar_widget_paint;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE(traian): Editor state and update cycle.
////////////////////////////////////////////////////////////////////////////////////////////////////

internal void
editor_build_widget_tree(EditorState *state)
{
    state->root_widget = ARENA_PUSH_STRUCT(state->memory->permanent_arena, PanelAssemblyWidget);
    PanelAssemblyWidget *panel_assembly_widget = (PanelAssemblyWidget *)state->root_widget;
    panel_assembly_widget_construct(panel_assembly_widget);
    panel_assembly_widget->parent = NULL;

    // Initialize panels.
    panel_assembly_widget->panel_count = 2;
    panel_assembly_widget->panels = ARENA_PUSH_ARRAY(state->memory->permanent_arena, PanelWidget, panel_assembly_widget->panel_count);
    panel_assembly_widget->child_count = panel_assembly_widget->panel_count;
    panel_assembly_widget->children = ARENA_PUSH_ARRAY(state->memory->permanent_arena, EditorWidget *, panel_assembly_widget->child_count);

    for (u32 panel_index = 0; panel_index < panel_assembly_widget->panel_count; ++panel_index) {
        PanelWidget *panel_widget = panel_assembly_widget->panels + panel_index;
        panel_assembly_widget->children[panel_index] = panel_widget;

        panel_widget_construct(panel_widget);
        panel_widget->parent = panel_assembly_widget;
        panel_widget->panel_index = panel_index;

        panel_widget->child_count = 2;
        panel_widget->children = ARENA_PUSH_ARRAY(state->memory->permanent_arena, EditorWidget *, panel_widget->child_count);
        panel_widget->children[0] = &panel_widget->content_buffer_widget;
        panel_widget->children[1] = &panel_widget->titlebar_widget;

        panel_widget->content_buffer_widget.parent = panel_widget;
        panel_widget->content_buffer_widget.child_count = 0;
        panel_widget->content_buffer_widget.children = NULL;
        panel_content_buffer_widget_construct(&panel_widget->content_buffer_widget);

        panel_widget->titlebar_widget.parent = panel_widget;
        panel_widget->titlebar_widget.child_count = 0;
        panel_widget->titlebar_widget.children = NULL;
        panel_titlebar_widget_construct(&panel_widget->titlebar_widget);
    }
}

internal void
editor_initialize_fonts(EditorState *state)
{
    FileReadResult ttf_result = platform_read_entire_file_to_arena("C:/Windows/Fonts/consola.ttf", state->memory->work_arena);
    ASSERT(ttf_result.is_valid);
    
    font_initialize(
        state->fonts + FONT_ID_DEFAULT, state->memory->permanent_arena,
        ttf_result.file_data, ttf_result.file_size, 30.0F);
}

function EditorState *
editor_initialize(EditorMemory *memory, Bitmap *offscreen_bitmap)
{
    EditorState *state = ARENA_PUSH_STRUCT(memory->permanent_arena, EditorState);
    state->memory = memory;
    state->offscreen_bitmap = offscreen_bitmap;

    editor_initialize_fonts(state);
    editor_build_widget_tree(state);

    // NOTE: Triggers an initialize event to propagate.
    state->root_widget->initialize(state, state->root_widget);
    editor_resize(state, offscreen_bitmap->size_x, offscreen_bitmap->size_y);

    return state;
}

function void
editor_resize(EditorState *state, u32 new_size_x, u32 new_size_y)
{
    // NOTE: Triggers a resize event to propagate.
    state->root_widget->resize(state, state->root_widget);
}

function void
editor_update(EditorState *state)
{
    // NOTE: Triggers an update event to propagate.
    state->root_widget->update(state, state->root_widget);

    // NOTE: Triggers a paint event to propagate.
    state->root_widget->paint(state, state->root_widget);
}

function void
editor_destroy(EditorState *state)
{ }
