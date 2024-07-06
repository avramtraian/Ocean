/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#include <Core/Assertion.h>
#include <Platform/Memory.h>

#if !OCEAN_PLATFORM_WINDOWS
#    error Trying to include the Windows headers but they are not available!
#endif // OCEAN_PLATFORM_WINDOWS

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

void* platform_memory_allocate(usize byte_count)
{
    if (byte_count == 0)
        return nullptr;

    void* memory_block = VirtualAlloc(nullptr, byte_count, MEM_COMMIT, PAGE_READWRITE);
    VERIFY(memory_block);
    return memory_block;
}

void platform_memory_release(void* memory_block, usize byte_count)
{
    if (byte_count == 0)
        return;

    VirtualFree(memory_block, 0, MEM_RELEASE);
    (void)byte_count;
}
