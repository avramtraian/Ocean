/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#pragma once

#include <Core/Memory/MemorySpan.h>
#include <Core/Types.h>

enum FileReadReturnCodeEnum {
    FILE_READ_RETURN_CODE_SUCCESS = 0,
    FILE_READ_RETURN_CODE_UNKOWN = 1,
    FILE_READ_RETURN_CODE_FILE_NOT_EXISTING,
    FILE_READ_RETURN_CODE_BUFFER_TOO_SMALL,
};
typedef u8 FileReadReturnCode;

typedef struct FileReadResult {
    FileReadReturnCode return_code;
    // Always set to INVALID_SIZE if the return code is not success.
    usize read_byte_count;
} FileReadResult;

// Returns INVALID_SIZE if the file doesn't exist or can't be accessed.
usize filesystem_get_file_size(const char* filename);

FileReadResult filesystem_read_entire_file(const char* filename, ReadWriteByteSpan read_buffer);
