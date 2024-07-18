/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#pragma once

#include <Core/Defines.h>
#include <Platform/Windows/WindowsHeaders.h>

#if OCEAN_CONFIGURATION_DEBUG

int main()
{
    // NOTE: The Ocean::guraded_main() function is expected to be declared before this file is included.
    const auto return_code = guarded_main();
    return (INT)(return_code);
}

#else

INT WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    // NOTE: The Ocean::guraded_main() function is expected to be declared before this file is included.
    const auto return_code = guarded_main();
    return (INT)(return_code);
}

#endif // OCEAN_CONFIGURATION_DEBUG
