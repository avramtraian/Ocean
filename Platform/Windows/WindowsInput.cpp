/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#include <Platform/Input.h>
#include <Platform/Windows/WindowsHeaders.h>

KeyModifierBits platform_input_get_key_modifier_bits()
{
    KeyModifierBits key_modifier_bits = KEY_MODIFIER_BIT_NONE;

    if (GetKeyState(VK_LCONTROL) & 0x8000 || GetKeyState(VK_RCONTROL) & 0x8000)
        key_modifier_bits |= KEY_MODIFIER_BIT_CTRL;

    if (GetKeyState(VK_LSHIFT) & 0x8000 || GetKeyState(VK_RSHIFT) & 0x8000)
        key_modifier_bits |= KEY_MODIFIER_BIT_SHIFT;

    if (GetKeyState(VK_LMENU) & 0x8000 || GetKeyState(VK_RMENU) & 0x8000)
        key_modifier_bits |= KEY_MODIFIER_BIT_ALT;

    return key_modifier_bits;
}

bool platform_input_is_caps_lock_active()
{
    return GetKeyState(VK_CAPITAL) & 1;
}

bool platform_input_is_num_lock_active()
{
    return GetKeyState(VK_NUMLOCK) & 1;
}
