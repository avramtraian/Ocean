/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#pragma once

#include <Core/Defines.h>

#if !OCEAN_PLATFORM_WINDOWS
#    error Trying to include the Windows headers but they are not available on the current platform!
#else
#    define WIN32_LEAN_AND_MEAN
#    define NOMINMAX
#    include <Windows.h>
#endif // !OCEAN_PLATFORM_WINDOWS
