/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#include <Core/Math/MathUtils.h>
#include <Platform/FileSystem.h>
#include <Platform/Windows/WindowsHeaders.h>

usize filesystem_get_file_size(const char* filename)
{
    HANDLE file_handle = CreateFileA(filename, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file_handle == INVALID_HANDLE_VALUE)
        return INVALID_SIZE;

    LARGE_INTEGER file_size;
    const BOOL file_size_success = GetFileSizeEx(file_handle, &file_size);
    CloseHandle(file_handle);

    return file_size_success ? file_size.QuadPart : INVALID_SIZE;
}

FileReadResult filesystem_read_entire_file(const char* filename, ReadWriteByteSpan read_buffer)
{
    FileReadResult result;
    result.return_code = FILE_READ_RETURN_CODE_UNKOWN;
    result.read_byte_count = INVALID_SIZE;

    HANDLE file_handle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file_handle == INVALID_HANDLE_VALUE) {
        result.return_code = FILE_READ_RETURN_CODE_FILE_NOT_EXISTING;
        return result;
    }

    LARGE_INTEGER file_size;
    if (!GetFileSizeEx(file_handle, &file_size)) {
        CloseHandle(file_handle);
        return result;
    }

    if (file_size.QuadPart > read_buffer.count) {
        CloseHandle(file_handle);
        result.return_code = FILE_READ_RETURN_CODE_BUFFER_TOO_SMALL;
        return result;
    }

    usize bytes_read_until_now = 0;
    while (bytes_read_until_now < file_size.QuadPart) {
        const u32 bytes_to_read = min_usize(file_size.QuadPart - bytes_read_until_now, (DWORD)(-1));
        DWORD bytes_read;
        if (!ReadFile(file_handle, read_buffer.bytes, bytes_to_read, &bytes_read, NULL))
            break;
        if (bytes_to_read != bytes_read)
            break;

        bytes_read_until_now += bytes_read;
    }

    CloseHandle(file_handle);

    if (bytes_read_until_now != file_size.QuadPart)
        return result;

    result.return_code = FILE_READ_RETURN_CODE_SUCCESS;
    result.read_byte_count = bytes_read_until_now;
    return result;
}
