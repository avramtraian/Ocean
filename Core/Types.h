/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#pragma once

#include <Core/Defines.h>

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long long;

using i8 = signed char;
using i16 = signed short;
using i32 = signed int;
using i64 = signed long long;

using usize = u64;
using ssize = i64;
using uintptr = u64;
using intptr = i64;

using ReadonlyByte = const u8;
using WriteonlyByte = u8;
using ReadWriteByte = u8;

using ReadonlyBytes = ReadonlyByte*;
using WriteonlyBytes = WriteonlyByte*;
using ReadWriteBytes = ReadWriteByte*;

#define INVALID_SIZE              ((usize)-1)
#define INVALID_HANDLE            ((void*)0)
#define INVALID_UNICODE_CODEPOINT ((u32)-1)

#define MAX_UINT8  ((u8)0xFF)
#define MAX_UINT16 ((u16)0xFFFF)
#define MAX_UINT32 ((u32)0xFFFFFFFF)
#define MAX_UINT64 ((u64)0xFFFFFFFFFFFFFFFF)

#define MAX_INT8  ((i8)0x7F)
#define MAX_INT16 ((i16)0x7FFF)
#define MAX_INT32 ((i32)0x7FFFFFFF)
#define MAX_INT64 ((i64)0x7FFFFFFFFFFFFFFF)

#define MIN_INT8  ((i8)-0x80)
#define MIN_INT16 ((i16)-0x8000)
#define MIN_INT32 ((i32)-0x80000000)
#define MIN_INT64 ((i64)-0x8000000000000000)
