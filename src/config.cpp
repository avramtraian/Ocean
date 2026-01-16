/*
 * Copyright (c) 2025-2026 Traian Avram. All rights reserved.
 * This file is part of my personal text editor and is distributed under the MIT license.
 */

constant s32 WRAP_SYMBOL_PADDING_SIZE = 6;
constant s32 TITLEBAR_SIZE            = 30;
constant s32 SCROLLBAR_SIZE           = 15;
constant s32 SPLITTER_SIZE            = 8;
constant s32 TAB_SIZE                 = 4;

constant f32 CURSOR_SIZE_PERCENTAGE_X = 0.2F;
constant f32 CURSOR_SIZE_PERCENTAGE_Y = 1.3F;

constant f32 START_WRAP_SYMBOL_SIZE_PERCENTAGE_Y = 0.5F;
constant f32 END_WRAP_SYMBOL_SIZE_PERCENTAGE_Y   = 0.5F;
const LinearColor START_WRAP_SYMBOL_COLOR        = linear_color(255, 255, 0);
const LinearColor END_WRAP_SYMBOL_COLOR          = linear_color(0, 0, 255);

const LinearColor FOREGROUND_COLOR           = linear_color(200, 200, 165);
const LinearColor BACKGROUND_COLOR           = linear_color(4, 36, 40);
const LinearColor BACKGROUND_SELECTED_COLOR  = linear_color(15, 30, 220);
const LinearColor TITLEBAR_BACKGROUND_COLOR  = linear_color(189, 180, 98);
const LinearColor TITLEBAR_FOREGROUND_COLOR  = linear_color(25, 25, 25);
const LinearColor SPLITTER_COLOR             = linear_color(189, 180, 98);
const LinearColor SCROLLBAR_BACKGROUND_COLOR = linear_color(210, 210, 210);
const LinearColor SCROLLBAR_FOREGROUND_COLOR = linear_color(150, 150, 150);
const LinearColor SCROLLBAR_HOVERED_COLOR    = linear_color(140, 140, 140);
const LinearColor SCROLLBAR_IN_USE_COLOR     = linear_color(120, 120, 120);
const LinearColor CURSOR_COLOR               = linear_color(220, 220, 220);
