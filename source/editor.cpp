/**
 * Copyright (c) 2025 Traian Avram. All rights reserved.
 * This source file is part of the Ocean text editor.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#include "core.h"
#include "draw.h"
#include "editor.h"

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
    const u32 titlebar_height = 40;
    widget->surface_offset_x = widget->parent->surface_offset_x;
    widget->surface_offset_y = widget->parent->surface_offset_y + titlebar_height;
    widget->surface_size_x = widget->parent->surface_size_x;
    widget->surface_size_y = max_s32((s32)widget->parent->surface_size_y - (s32)titlebar_height, 0);

    editor_widget_resize(state, widget);
}

internal EDITOR_WIDGET_API_RESIZE(panel_titlebar_widget_resize)
{
    const u32 titlebar_height = 40;
    widget->surface_offset_x = widget->parent->surface_offset_x;
    widget->surface_offset_y = widget->parent->surface_offset_y;
    widget->surface_size_x = widget->parent->surface_size_x;
    widget->surface_size_y = min_u32(titlebar_height, widget->parent->surface_size_y);

    editor_widget_resize(state, widget);
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
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE(traian): Widget construction.
////////////////////////////////////////////////////////////////////////////////////////////////////

internal EDITOR_WIDGET_API_CONSTRUCT(editor_widget_construct)
{
    widget->initialize = NULL;
    widget->resize = editor_widget_resize;
    widget->update = NULL;
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
    widget->resize = panel_titlebar_widget_resize;
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
    state->root_widget->paint(state, state->root_widget);
}

function void
editor_destroy(EditorState *state)
{ }
