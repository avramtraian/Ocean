/*
 * Copyright (c) 2025-2026 Traian Avram. All rights reserved.
 * This file is part of my personal text editor and is distributed under the MIT license.
 */

struct Utf8DecodeResult {
    bool is_valid;
    u32 codepoint;
    usize byte_width;
};

struct Utf8EncodeResult {
    bool is_valid;
    u8 data[4];
    usize byte_width;
};

// https://www.ietf.org/rfc/rfc3629.txt
// +---------------------+-------------------------------------+
// | Char. number range  |         UTF-8 octet sequence        |
// |    (hexadecimal)    |               (binary)              |
// +---------------------+-------------------------------------+
// | 0000 0000-0000 007F | 0xxxxxxx                            |
// | 0000 0080-0000 07FF | 110xxxxx 10xxxxxx                   |
// | 0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx          |
// | 0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx |
// +---------------------+-------------------------------------+

#define UTF8_CHECK_FOR_ONE_BYTE_SEQUENCE(byte) \
    (((byte) & 0b1'0000000) == 0b0'0000000)

#define UTF8_CHECK_FOR_TWO_BYTES_SEQUENCE(byte) \
    (((byte) & 0b111'00000) == 0b110'00000)

#define UTF8_CHECK_FOR_THREE_BYTES_SEQUENCE(byte) \
    (((byte) & 0b1111'0000) == 0b1110'0000)

#define UTF8_CHECK_FOR_FOUR_BYTES_SEQUENCE(byte) \
    (((byte) & 0b11111'000) == 0b11110'000)

#define UTF8_VALIDATE_CONTINUATION_BYTE(byte)     \
    if (((byte) & 0b11'000000) != 0b10'000000) {  \
        return {};                                \
    }

#define UTF8_VALIDATE_ENOUGH_BYTES(byte_count, minimum_expected) \
    if ((byte_count) < (minimum_expected)) {                     \
        return {};                                               \
    }

internal inline Utf8DecodeResult
utf8_decode_one_byte(u8 byte_0)
{
    // The encoded sequence: 0xxxxxxx.
    u32 codepoint = 0;
    codepoint |= (byte_0 & 0b0'1111111) << 0;

    // The decoded code point must be in the specification range.
    if (codepoint < 0x0000'0000 || codepoint > 0x0000'007F)
        return {};

    Utf8DecodeResult result;
    result.is_valid = true;
    result.codepoint = codepoint;
    result.byte_width = 1;
    return result;
}

internal inline Utf8DecodeResult
utf8_decode_two_bytes(u8 byte_0, u8 byte_1)
{
    // The encoded sequence: 110xxxxx 10xxxxxx.
    u32 codepoint = 0;
    codepoint |= (byte_0 & 0b000'11111) << 6;
    codepoint |= (byte_1 & 0b00'111111) << 0;

    // The decoded code point must be in the specification range.
    if (codepoint < 0x0000'0080 || codepoint > 0x0000'07FF)
        return {};

    Utf8DecodeResult result;
    result.is_valid = true;
    result.codepoint = codepoint;
    result.byte_width = 2;
    return result;
}

internal inline Utf8DecodeResult
utf8_decode_three_bytes(u8 byte_0, u8 byte_1, u8 byte_2)
{
    // The encoded sequence: 1110xxxx 10xxxxxx 10xxxxxx.
    u32 codepoint = 0;
    codepoint |= (byte_0 & 0b0000'1111) << 12;
    codepoint |= (byte_1 & 0b00'111111) << 6;
    codepoint |= (byte_2 & 0b00'111111) << 0;

    // The decoded code point must be in the specification range.
    if (codepoint < 0x0000'0800 || codepoint > 0x0000'FFFF)
        return {};

    Utf8DecodeResult result;
    result.is_valid = true;
    result.codepoint = codepoint;
    result.byte_width = 3;
    return result;
}

internal inline Utf8DecodeResult
utf8_decode_four_bytes(u8 byte_0, u8 byte_1, u8 byte_2, u8 byte_3)
{
    // The encoded sequence: 1110xxxx 10xxxxxx 10xxxxxx.
    u32 codepoint = 0;
    codepoint |= (byte_0 & 0b00000'111) << 18;
    codepoint |= (byte_1 & 0b00'111111) << 12;
    codepoint |= (byte_2 & 0b00'111111) << 6;
    codepoint |= (byte_3 & 0b00'111111) << 0;

    // The decoded code point must be in the specification range.
    if (codepoint < 0x0001'0000 || codepoint > 0x0010'FFFF)
        return {};

    Utf8DecodeResult result;
    result.is_valid = true;
    result.codepoint = codepoint;
    result.byte_width = 4;
    return result;
}

internal Utf8DecodeResult
utf8_decode(void* data, usize size)
{
    UTF8_VALIDATE_ENOUGH_BYTES(size, 1);
    u8* bytes      = (u8*)data;
    u8  first_byte = bytes[0];

    if (UTF8_CHECK_FOR_ONE_BYTE_SEQUENCE(first_byte)) {
        UTF8_VALIDATE_ENOUGH_BYTES(size, 1);
        return utf8_decode_one_byte(bytes[0]);
    }

    if (UTF8_CHECK_FOR_TWO_BYTES_SEQUENCE(first_byte)) {
        UTF8_VALIDATE_ENOUGH_BYTES(size, 2);
        UTF8_VALIDATE_CONTINUATION_BYTE(bytes[1]);
        return utf8_decode_two_bytes(bytes[0], bytes[1]);
    }

    if (UTF8_CHECK_FOR_THREE_BYTES_SEQUENCE(first_byte)) {
        UTF8_VALIDATE_ENOUGH_BYTES(size, 3);
        UTF8_VALIDATE_CONTINUATION_BYTE(bytes[1]);
        UTF8_VALIDATE_CONTINUATION_BYTE(bytes[2]);
        return utf8_decode_three_bytes(bytes[0], bytes[1], bytes[2]);
    }

    if (UTF8_CHECK_FOR_FOUR_BYTES_SEQUENCE(first_byte)) {
        UTF8_VALIDATE_ENOUGH_BYTES(size, 4);
        UTF8_VALIDATE_CONTINUATION_BYTE(bytes[1]);
        UTF8_VALIDATE_CONTINUATION_BYTE(bytes[2]);
        UTF8_VALIDATE_CONTINUATION_BYTE(bytes[3]);
        return utf8_decode_four_bytes(bytes[0], bytes[1], bytes[2], bytes[3]);
    }

    // The first byte is not correctly encoded as UTF-8.
    return {};
}

internal Utf8DecodeResult
utf8_decode_reversed(void* data, usize size)
{
    UTF8_VALIDATE_ENOUGH_BYTES(size, 1);
    u8* bytes = (u8*)data + size;

    if (UTF8_CHECK_FOR_ONE_BYTE_SEQUENCE(bytes[-1]))
        return utf8_decode_one_byte(bytes[-1]);

    UTF8_VALIDATE_CONTINUATION_BYTE(bytes[-1]);
    UTF8_VALIDATE_ENOUGH_BYTES(size, 2);

    if (UTF8_CHECK_FOR_TWO_BYTES_SEQUENCE(bytes[-2]))
        return utf8_decode_two_bytes(bytes[-2], bytes[-1]);

    UTF8_VALIDATE_CONTINUATION_BYTE(bytes[-2]);
    UTF8_VALIDATE_ENOUGH_BYTES(size, 3);

    if (UTF8_CHECK_FOR_THREE_BYTES_SEQUENCE(bytes[-3]))
        return utf8_decode_three_bytes(bytes[-3], bytes[-2], bytes[-1]);

    UTF8_VALIDATE_CONTINUATION_BYTE(bytes[-3]);
    UTF8_VALIDATE_ENOUGH_BYTES(size, 4);

    if (UTF8_CHECK_FOR_FOUR_BYTES_SEQUENCE(bytes[-4]))
        return utf8_decode_four_bytes(bytes[-4], bytes[-3], bytes[-2], bytes[-1]);

    return {};
}

// Assumes that 'size' is non-zero.
internal u32
utf8_decoded_or_raw_byte(void* data, usize size)
{
    ASSERT(size > 0);

    Utf8DecodeResult decode_result = utf8_decode(data, size);
    if (decode_result.is_valid)
        return decode_result.codepoint;

    u8 raw_byte = *(u8*)data;
    return raw_byte;
}

// Assumes that 'size' is non-zero.
internal u32
utf8_decoded_or_raw_byte_reversed(void* data, usize size)
{
    ASSERT(size > 0);

    Utf8DecodeResult decode_result = utf8_decode_reversed(data, size);
    if (decode_result.is_valid)
        return decode_result.codepoint;

    u8 raw_byte = *((u8*)data + size - 1);
    return raw_byte;
}

internal Utf8EncodeResult
utf8_encode(u32 codepoint)
{
    // https://www.ietf.org/rfc/rfc3629.txt
    // +---------------------+-------------------------------------+
    // | Char. number range  |         UTF-8 octet sequence        |
    // |    (hexadecimal)    |               (binary)              |
    // +---------------------+-------------------------------------+
    // | 0000 0000-0000 007F | 0xxxxxxx                            |
    // | 0000 0080-0000 07FF | 110xxxxx 10xxxxxx                   |
    // | 0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx          |
    // | 0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx |
    // +---------------------+-------------------------------------+

    if (0x0000'0000 <= codepoint && codepoint <= 0x0000'007F) {
        // The rune needs 1-byte to be encoded: 0xxxxxxx.
        Utf8EncodeResult encode_result;
        encode_result.is_valid = true;
        encode_result.byte_width = 1;
        encode_result.data[0] = 0b0'0000000 | ((codepoint >> 0) & 0b0'1111111);
        return encode_result;
    }

    if (0x0000'0080 <= codepoint && codepoint <= 0x0000'07FF) {
        // The rune needs 2-bytes to be encoded: 110xxxxx 10xxxxxx.
        Utf8EncodeResult encode_result;
        encode_result.is_valid = true;
        encode_result.byte_width = 2;
        encode_result.data[0] = 0b110'00000 | ((codepoint >> 6) & 0b000'11111);
        encode_result.data[1] = 0b10'000000 | ((codepoint >> 0) & 0b00'111111);
        return encode_result;
    }

    if (0x0000'0800 <= codepoint && codepoint <= 0x0000'FFFF) {
        // https://www.ietf.org/rfc/rfc3629.txt
        // The definition of UTF-8 prohibits encoding character numbers between U+D800 and U+DFFF, which are reserved
        // for use with the UTF-16 encoding form (as surrogate pairs) and do not directly represent characters.
        if (0xD800 <= codepoint && codepoint <= 0xDFFF)
            return {};

        // The rune needs 3-bytes to be encoded: 1110xxxx 10xxxxxx 10xxxxxx.
        Utf8EncodeResult encode_result;
        encode_result.is_valid = true;
        encode_result.byte_width = 3;
        encode_result.data[0] = 0b1110'0000 | ((codepoint >> 12) & 0b0000'1111);
        encode_result.data[1] = 0b10'000000 | ((codepoint >> 6)  & 0b00'111111);
        encode_result.data[2] = 0b10'000000 | ((codepoint >> 0)  & 0b00'111111);
        return encode_result;
    }

    if (0x0001'0000 <= codepoint && codepoint <= 0x0010'FFFF) {
        // The rune needs 4-bytes to be encoded: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx.
        Utf8EncodeResult encode_result;
        encode_result.is_valid = true;
        encode_result.byte_width = 4;
        encode_result.data[0] = 0b11110'000 | ((codepoint >> 18) & 0b00000'111);
        encode_result.data[1] = 0b10'000000 | ((codepoint >> 12) & 0b00'111111);
        encode_result.data[2] = 0b10'000000 | ((codepoint >> 6)  & 0b00'111111);
        encode_result.data[3] = 0b10'000000 | ((codepoint >> 0)  & 0b00'111111);
        return encode_result;
    }

    return {};
}

struct Utf8Iterator {
    u8* data;
    usize size;
    usize offset;
    Utf8DecodeResult last_decode_result;

    // Always equal to the corresponding fields of 'last_decode_result', and thus these contain valid data only
    // when the iterator is valid. Their only purpose is making the accessing API easier to use and read.
    u32 codepoint;
    usize byte_width;
};

internal Utf8Iterator
utf8_iterator(void* data, usize size)
{
    Utf8Iterator iterator = {};
    iterator.data = (u8*)data;
    iterator.size = size;
    iterator.offset = 0;
    iterator.last_decode_result = utf8_decode(iterator.data, iterator.size);
    if (iterator.last_decode_result.is_valid) {
        iterator.codepoint = iterator.last_decode_result.codepoint;
        iterator.byte_width = iterator.last_decode_result.byte_width;
    }
    return iterator;
}

internal Utf8Iterator
utf8_iterator(String string)
{
    Utf8Iterator result = utf8_iterator(string.data, string.size);
    return result;
}

internal bool
is_valid(Utf8Iterator iterator)
{
    bool result =  iterator.last_decode_result.is_valid;
    return result;
}

internal void
advance(Utf8Iterator* iterator)
{
    ASSERT(is_valid(*iterator));
    iterator->offset += iterator->last_decode_result.byte_width;
    iterator->last_decode_result = utf8_decode(iterator->data + iterator->offset, iterator->size - iterator->offset);
    if (iterator->last_decode_result.is_valid) {
        iterator->codepoint = iterator->last_decode_result.codepoint;
        iterator->byte_width = iterator->last_decode_result.byte_width;
    }
}

internal Utf8DecodeResult
peek_next(Utf8Iterator iterator)
{
    ASSERT(is_valid(iterator));
    advance(&iterator);
    return iterator.last_decode_result;
}

internal usize
utf8_get_codepoint_count(void* data, usize size)
{
    usize codepoint_count = 0;
    Utf8Iterator iterator = utf8_iterator(data, size);
    while (is_valid(iterator)) {
        ++codepoint_count;
        advance(&iterator);
    }
    return codepoint_count;
}

internal usize
utf8_get_codepoint_count(String string)
{
    usize result = utf8_get_codepoint_count(string.data, string.size);
    return result;
}
