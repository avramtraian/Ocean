/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#include <Core/Encoding/Utf8.h>

u32 utf8_bytes_to_codepoint(ReadonlyByteSpan byte_span, usize* out_codepoint_width)
{
    if (byte_span.count == 0) {
        *out_codepoint_width = 0;
        return INVALID_UNICODE_CODEPOINT;
    }

    if ((byte_span.bytes[0] & 0x80) == 0x00) {
        *out_codepoint_width = 1;
        return byte_span.bytes[0];
    }

    if ((byte_span.bytes[0] & 0xE0) == 0xC0) {
        if (byte_span.count < 2) {
            *out_codepoint_width = 0;
            return INVALID_UNICODE_CODEPOINT;
        }

        u32 codepoint = 0;
        codepoint += (byte_span.bytes[0] & 0x1F) << 6;
        codepoint += (byte_span.bytes[1] & 0x3F) << 0;

        *out_codepoint_width = 2;
        return codepoint;
    }

    if ((byte_span.bytes[0] & 0xF0) == 0xE0) {
        if (byte_span.count < 3) {
            *out_codepoint_width = 0;
            return INVALID_UNICODE_CODEPOINT;
        }

        u32 codepoint = 0;
        codepoint += (byte_span.bytes[0] & 0x1F) << 12;
        codepoint += (byte_span.bytes[1] & 0x3F) << 6;
        codepoint += (byte_span.bytes[2] & 0x3F) << 0;

        *out_codepoint_width = 3;
        return codepoint;
    }

    if ((byte_span.bytes[0] & 0xF8) == 0xF0) {
        if (byte_span.count < 4) {
            *out_codepoint_width = 0;
            return INVALID_UNICODE_CODEPOINT;
        }

        u32 codepoint = 0;
        codepoint += (byte_span.bytes[0] & 0x1F) << 18;
        codepoint += (byte_span.bytes[1] & 0x3F) << 12;
        codepoint += (byte_span.bytes[2] & 0x3F) << 6;
        codepoint += (byte_span.bytes[3] & 0x3F) << 0;

        *out_codepoint_width = 4;
        return codepoint;
    }

    *out_codepoint_width = 0;
    return INVALID_UNICODE_CODEPOINT;
}

usize utf8_bytes_to_codepoint_width(ReadonlyByteSpan byte_span)
{
    if (byte_span.count == 0)
        return 0;

    if ((byte_span.bytes[0] & 0x80) == 0x00)
        return 1;

    if ((byte_span.bytes[0] & 0xE0) == 0xC0) {
        if (byte_span.count < 2)
            return 0;
        return 2;
    }

    if ((byte_span.bytes[0] & 0xF0) == 0xE0) {
        if (byte_span.count < 3)
            return 0;
        return 3;
    }

    if ((byte_span.bytes[0] & 0xF8) == 0xF0) {
        if (byte_span.count < 4)
            return 0;
        return 4;
    }

    return 0;
}

usize utf8_bytes_from_codepoint(u32 codepoint, ReadWriteByteSpan destination_byte_span)
{
    if (codepoint <= 0x007F) {
        if (destination_byte_span.count < 1)
            return 0;
        destination_byte_span.bytes[0] = (ReadWriteByte)codepoint;
        return 1;
    }

    if (0x0080 <= codepoint && codepoint <= 0x07FF) {
        if (destination_byte_span.count < 2)
            return 0;
        destination_byte_span.bytes[0] = ((codepoint >> 6) & 0x1F) | 0xC0;
        destination_byte_span.bytes[1] = ((codepoint >> 0) & 0x3F) | 0x80;
        return 2;
    }

    if (0x0800 <= codepoint && codepoint <= 0xFFFF) {
        if (destination_byte_span.count < 3)
            return 0;
        destination_byte_span.bytes[0] = ((codepoint >> 12) & 0x0F) | 0xE0;
        destination_byte_span.bytes[1] = ((codepoint >> 6) & 0x3F) | 0x80;
        destination_byte_span.bytes[2] = ((codepoint >> 0) & 0x3F) | 0x80;
        return 3;
    }

    if (0x10000 <= codepoint) {
        if (destination_byte_span.count < 4)
            return 0;
        destination_byte_span.bytes[0] = ((codepoint >> 18) & 0x07) | 0xF0;
        destination_byte_span.bytes[1] = ((codepoint >> 12) & 0x3F) | 0x80;
        destination_byte_span.bytes[2] = ((codepoint >> 6) & 0x3F) | 0x80;
        destination_byte_span.bytes[3] = ((codepoint >> 0) & 0x3F) | 0x80;
        return 4;
    }

    return 0;
}

usize utf8_codepoint_width(u32 codepoint)
{
    if (codepoint <= 0x007F)
        return 1;
    if (0x0080 <= codepoint && codepoint <= 0x07FF)
        return 2;
    if (0x0800 <= codepoint && codepoint <= 0xFFFF)
        return 3;
    if (0x10000 <= codepoint)
        return 4;

    return 0;
}
