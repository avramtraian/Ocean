/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#pragma once

#include <Core/Types.h>

typedef struct ReadonlyByteSpan {
    ReadonlyBytes bytes;
    usize count;
} ReadonlyByteSpan;

typedef struct ReadWriteByteSpan {
    ReadWriteBytes bytes;
    usize count;
} ReadWriteByteSpan;

inline ReadonlyByteSpan readonly_byte_span(ReadonlyBytes bytes, usize count)
{
    ReadonlyByteSpan byte_span;
    byte_span.bytes = bytes;
    byte_span.count = count;
    return byte_span;
}

inline ReadWriteByteSpan readonly_byte_span(ReadWriteBytes bytes, usize count)
{
    ReadWriteByteSpan byte_span;
    byte_span.bytes = bytes;
    byte_span.count = count;
    return byte_span;
}
