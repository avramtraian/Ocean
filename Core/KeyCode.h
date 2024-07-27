/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#pragma once

#include <Core/Types.h>

enum KeyModifierBitsEnum {
    KEY_MODIFIER_BIT_NONE = 0,
    KEY_MODIFIER_BIT_CTRL = 1,
    KEY_MODIFIER_BIT_SHIFT = 2,
    KEY_MODIFIER_BIT_ALT = 4,
};
typedef u8 KeyModifierBits;

enum KeyCodeEnum {
    KEY_CODE_NONE = 0,
    KEY_CODE_MIN_VALUE,

    KEY_CODE_A,
    KEY_CODE_B,
    KEY_CODE_C,
    KEY_CODE_D,
    KEY_CODE_E,
    KEY_CODE_F,
    KEY_CODE_G,
    KEY_CODE_H,
    KEY_CODE_I,
    KEY_CODE_J,
    KEY_CODE_K,
    KEY_CODE_L,
    KEY_CODE_M,
    KEY_CODE_N,
    KEY_CODE_O,
    KEY_CODE_P,
    KEY_CODE_Q,
    KEY_CODE_R,
    KEY_CODE_S,
    KEY_CODE_T,
    KEY_CODE_U,
    KEY_CODE_V,
    KEY_CODE_W,
    KEY_CODE_X,
    KEY_CODE_Y,
    KEY_CODE_Z,

    KEY_CODE_MAX_VALUE
};
typedef u16 KeyCode;

enum MouseButtonEnum {
    MOUSE_BUTTON_NONE = 0,
    MOUSE_BUTTON_MIN_VALUE,

    MOUSE_BUTTON_LEFT,
    MOUSE_BUTTON_MIDDLE,
    MOUSE_BUTTON_RIGHT,

    MOUSE_BUTTON_MAX_VALUE,
};
typedef u8 MouseButton;
