/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#pragma once

#include <Core/Defines.h>

#define VERIFY(...)       \
    if (!(__VA_ARGS__)) { \
        OCEAN_DEBUGBREAK; \
    }
