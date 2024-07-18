/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#include <Editor/Settings.h>

void editor_settings_initialize_default_color_scheme(EditorColorScheme* color_scheme)
{
    color_scheme->background = linear_color(12, 12, 12);
}

void editor_settings_initialize_default(EditorSettings* editor_settings)
{
    editor_settings_initialize_default_color_scheme(&editor_settings->color_scheme);
}
