/*
 * Copyright (c) 2024 Traian Avram. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#include <Core/Log.h>

#include <cstdarg>
#include <cstdio>

static char s_debug_message_buffer[2048];
static char s_warning_message_buffer[2048];
static char s_error_message_buffer[2048];

void dbgln(const char* format, ...)
{
    snprintf(s_debug_message_buffer, sizeof(s_debug_message_buffer), "DEBUG: %s\n", format);

    va_list format_arguments;
    va_start(format_arguments, format);
    va_end(format_arguments);
    vprintf(s_debug_message_buffer, format_arguments);
}

void warnln(const char* format, ...)
{
    snprintf(s_warning_message_buffer, sizeof(s_warning_message_buffer), "WARN:  %s\n", format);

    va_list format_arguments;
    va_start(format_arguments, format);
    va_end(format_arguments);
    vprintf(s_warning_message_buffer, format_arguments);
}

void errorln(const char* format, ...)
{
    snprintf(s_error_message_buffer, sizeof(s_error_message_buffer), "ERROR: %s\n", format);

    va_list format_arguments;
    va_start(format_arguments, format);
    va_end(format_arguments);
    vprintf(s_error_message_buffer, format_arguments);
}
