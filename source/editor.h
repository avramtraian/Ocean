/**
 * Copyright (c) 2025 Traian Avram. All rights reserved.
 * This source file is part of the Ocean text editor.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#pragma once

#include "core.h"
#include "graphics.h"

// Forward declarations.
struct EditorMemory;
struct EditorState;
struct EditorWidget;

////////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE(traian): Editor state.
////////////////////////////////////////////////////////////////////////////////////////////////////

struct EditorMemory
{
    MemoryArena *permanent_arena;
    MemoryArena *work_arena;
    MemoryArena *dynamic_resources_arena;
};

enum FontIDEnum : u8
{
    FONT_ID_DEFAULT = 0,
    FONT_ID_MAX_COUNT,
};
typedef u16 FontID;

struct EditorState
{
    EditorMemory *memory;
    Bitmap       *offscreen_bitmap;
    Font          fonts[FONT_ID_MAX_COUNT];
    EditorWidget *root_widget;
};

function EditorState * editor_initialize(EditorMemory *memory, Bitmap *offscreen_bitmap);
function void          editor_resize    (EditorState *state, u32 new_size_x, u32 new_size_y);
function void          editor_update    (EditorState *state);
function void          editor_destroy   (EditorState *state);

////////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE(traian): Editor widgets.
////////////////////////////////////////////////////////////////////////////////////////////////////

#define EDITOR_WIDGET_API_CONSTRUCT(name) void name(EditorWidget *widget)

#define EDITOR_WIDGET_API_INITIALIZE(name) void name(EditorState *state, EditorWidget *widget)
typedef EDITOR_WIDGET_API_INITIALIZE(EditorWidgetInitialize);
function EDITOR_WIDGET_API_INITIALIZE(editor_widget_initialize);

#define EDITOR_WIDGET_API_RESIZE(name) void name(EditorState *state, EditorWidget *widget)
typedef EDITOR_WIDGET_API_RESIZE(EditorWidgetResize);
function EDITOR_WIDGET_API_RESIZE(editor_widget_resize);

#define EDITOR_WIDGET_API_UPDATE(name) void name(EditorState *state, EditorWidget *widget)
typedef EDITOR_WIDGET_API_UPDATE(EditorWidgetUpdate);
function EDITOR_WIDGET_API_UPDATE(editor_widget_update);

#define EDITOR_WIDGET_API_PAINT(name) void name(EditorState *state, EditorWidget *widget)
typedef EDITOR_WIDGET_API_PAINT(EditorWidgetPaint);
function EDITOR_WIDGET_API_PAINT(editor_widget_paint);

struct EditorWidget
{
    EditorWidget           *parent;
    EditorWidget          **children;
    u32                     child_count;
    s32                     surface_offset_x;
    s32                     surface_offset_y;
    u32                     surface_size_x;
    u32                     surface_size_y;
    EditorWidgetInitialize *initialize;
    EditorWidgetResize     *resize;
    EditorWidgetUpdate     *update;
    EditorWidgetPaint      *paint;
};

struct PanelContentBufferWidget : public EditorWidget
{
};

struct PanelTitlebarWidget : public EditorWidget
{
    char           *title_buffer;
    usize           title_buffer_size;
    String          title;
    u32             line_number;
    u32             column_number;
    TiledTextBuffer text_buffer;
};

struct PanelWidget : public EditorWidget
{
    u32                      panel_index;
    PanelContentBufferWidget content_buffer_widget;
    PanelTitlebarWidget      titlebar_widget;
};

struct PanelAssemblyWidget : public EditorWidget
{
    PanelWidget *panels;
    u32          panel_count;
};
