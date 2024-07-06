/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#pragma once

//=====================
// PLATFORM DETECTION.
//=====================

#ifdef _WIN32
#    ifndef _WIN64
#        error 32-bit architectures are not supported!
#    endif // _WIN64
#    define OCEAN_PLATFORM_WINDOWS 1
#endif // _WIN32

#ifndef OCEAN_PLATFORM_WINDOWS
#    define OCEAN_PLATFORM_WINDOWS 0
#endif // OCEAN_PLATFORM_WINDOWS

#ifndef OCEAN_PLATFORM_LINUX
#    define OCEAN_PLATFORM_LINUX 0
#endif // OCEAN_PLATFORM_LINUX

#ifndef OCEAN_PLATFORM_MACOS
#    define OCEAN_PLATFORM_MACOS 0
#endif // OCEAN_PLATFORM_MACOS

#if !OCEAN_PLATFORM_WINDOWS && !OCEAN_PLATFORM_LINUX && !OCEAN_PLATFORM_MACOS
#    error Unknown or unsupported platform!
#endif // Any platform.

//=====================
// COMPILER DETECTION.
//=====================

#ifdef __clang__
#    define OCEAN_COMPILER_CLANG 1
#else
#    ifdef _MSC_BUILD
#        define OCEAN_COMPILER_MSVC 1
#    endif // _MSC_BUILD
#    ifdef __GNUC__
#        define OCEAN_COMPILER_GCC 1
#    endif // __GNUC__
#endif // __clang__

#ifndef OCEAN_COMPILER_MSVC
#    define OCEAN_COMPILER_MSVC 0
#endif // OCEAN_COMPILER_MSVC

#ifndef OCEAN_COMPILER_CLANG
#    define OCEAN_COMPILER_CLANG 0
#endif // OCEAN_COMPILER_CLANG

#ifndef OCEAN_COMPILER_GCC
#    define OCEAN_COMPILER_GCC 0
#endif // OCEAN_COMPILER_GCC

#if !OCEAN_COMPILER_MSVC && !OCEAN_COMPILER_CLANG && !OCEAN_COMPILER_GCC
#    error Unknown or unsupported compiler!
#endif // Any compiler.

//==========================
// CONFIGURATION DETECTION.
//==========================

#ifdef _DEBUG
#    define OCEAN_CONFIGURATION_DEBUG   1
#    define OCEAN_CONFIGURATION_RELEASE 0
#else
#    define OCEAN_CONFIGURATION_DEBUG   0
#    define OCEAN_CONFIGURATION_RELEASE 1
#endif // _DEBUG

//===================================
// COMPILER-SPECIFIC UTILITY MACROS.
//===================================

#if OCEAN_COMPILER_MSVC
#    define ALWAYS_INLINE           __forceinline
#    define OFFSETOF(klass, member) ((::size_t) & reinterpret_cast<char const volatile&>((((s*)0)->m)))
#    define OCEAN_DEBUGBREAK        __debugbreak()
#    define OCEAN_FUNCTION          __FUNCSIG__
#else
#    define ALWAYS_INLINE           inline __attribute__((always_inline))
#    define OFFSETOF(klass, member) __builtin_offsetof(klass, member)
#    define OCEAN_DEBUGBREAK        __builtin_trap()
#    define OCEAN_FUNCTION          __PRETTY_FUNCTION__
#endif // OCEAN_COMPILER_MSVC

#define NODISCARD  [[nodiscard]]
#define OCEAN_LINE __LINE__
#define OCEAN_FILE __FILE__
#define OCEAN_DATE __DATE__

//==================================
// GENERAL PUROPOSE UTILITY MACROS.
//==================================

#define KiB ((unsigned long long)(1024))
#define MiB (1024 * KiB)
#define GiB (1024 * MiB)

#define ARRAY_COUNT(array) (sizeof(array) / sizeof(array[0]))
