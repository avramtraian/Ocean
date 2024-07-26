/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#pragma once

#include <Core/Memory/MemorySpan.h>
#include <Core/Types.h>

u32 utf8_bytes_to_codepoint(ReadonlyByteSpan byte_span, usize& out_codepoint_width);

usize utf8_bytes_to_codepoint_width(ReadonlyByteSpan byte_span);

usize utf8_bytes_from_codepoint(u32 codepoint, ReadWriteByteSpan destination_byte_span);

usize utf8_codepoint_width(u32 codepoint);
