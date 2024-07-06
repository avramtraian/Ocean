/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#pragma once

#include <Core/Defines.h>

#if !OCEAN_PLATFORM_WINDOWS
#    error Trying to include the Windows headers but they are not available!
#endif // OCEAN_PLATFORM_WINDOWS

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

INT WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    // NOTE: The Ocean::guraded_main() function is expected to be declared before this file is included.
    const auto return_code = Ocean::guarded_main();
    return static_cast<INT>(return_code);
}
