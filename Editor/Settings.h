/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#pragma once

#include <Core/Math/Color.h>

typedef struct EditorColorScheme {
    LinearColor background;
} EditorColorScheme;

typedef struct EditorSettings {
    EditorColorScheme color_scheme;
} EditorSettings;

void editor_settings_initialize_default_color_scheme(EditorColorScheme* color_scheme);

// NOTE: The color scheme will be initialized using 'editor_settings_initialize_default_color_scheme'.
void editor_settings_initialize_default(EditorSettings* editor_settings);
