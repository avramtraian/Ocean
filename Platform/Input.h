/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#pragma once

#include <Core/KeyCode.h>

KeyModifierBits platform_input_get_key_modifier_bits();

bool platform_input_is_caps_lock_active();

bool platform_input_is_num_lock_active();
