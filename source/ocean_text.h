/*  =====================================================================
    $File:   ocean_text.h $
    $Date:   October 2 2023 $
    $Author: Traian Avram $
    $Notice: Copyright (c) 2023-2023 Traian Avram. All Rights Reserved. $
    =====================================================================  */
#ifndef OCEAN_TEXT_H

#include "ocean.h"

struct text_iterator
{
    u32 Codepoint;
    u32 Width;
    memory_offset Offset;
    text_buffer *Buffer;
};

struct get_codepoint_result
{
    u32 Codepoint;
    u32 Width;
    b32 IsValid;
};

internal inline get_codepoint_result
GetCodepoint(text_buffer *Buffer, memory_offset Offset)
{
    Assert(Offset <= Buffer->Used);
    get_codepoint_result Result = {};
    
    if (Offset < Buffer->Used)
    {
        // TODO(traian): Support for Unicode encodings, such as UTF-8 or UTF-16!
        //               Currently, this code will cause a crash if the file is not plain ASCII.
        Result.Codepoint = Buffer->Base[Offset];
        Result.Width = 1;
        Result.IsValid = true;

        // NOTE(traian): On Windows, new lines are denoted using the '\r\n' sequence.
        //               This detail should not matter to the code that uses text iterators,
        //               so we abstract it by combining the two ASCII bytes into a single
        //               codepoint equal to '\n', but of width 2.
        if (Result.Codepoint == '\r')
        {
            if (Offset + Result.Width < Buffer->Used)
            {
                u8 NextByte = Buffer->Base[Offset + 1];
                if (NextByte == '\n')
                {
                    Result.Codepoint = '\n';
                    Result.Width = 2;
                }
            }
        }
    }
    else
    {
        Result.IsValid = true;
    }

    return Result;
}

internal inline text_iterator
NewTextIterator(text_buffer *Buffer, memory_offset Offset)
{
    text_iterator Result = {};

    get_codepoint_result Codepoint = GetCodepoint(Buffer, Offset);
    if (Codepoint.IsValid)
    {
        Result.Codepoint = Codepoint.Codepoint;
        Result.Width = Codepoint.Width;
        Result.Offset = Offset;
        Result.Buffer = Buffer;
    }

    return Result;
}

internal inline text_iterator
AdvanceIterator(text_iterator Iterator)
{
    text_iterator Result = {};
    memory_offset NewOffset = Iterator.Offset + Iterator.Width;
    if (NewOffset < Iterator.Buffer->Used)
    {
        Result = NewTextIterator(Iterator.Buffer, NewOffset);
    }

    return Result;
}

internal inline text_iterator
DevanceIterator(text_iterator Iterator)
{
    text_iterator Result = {};

    memory_offset Offset = Iterator.Offset;
    while (Offset)
    {
        get_codepoint_result Codepoint = GetCodepoint(Iterator.Buffer, --Offset);
        if (Codepoint.IsValid)
        {
            // NOTE(traian): Consume both of the bytes if the new line sequence is '\r\n'.
            //               Used when iterating over files that use CRLF new lines.
            if (Codepoint.Codepoint == '\n')
            {
                if (Offset > 0 && Iterator.Buffer->Base[Offset - 1] == '\r')
                {
                    --Offset;
                    Codepoint.Width++;
                }
            }

            Result.Codepoint = Codepoint.Codepoint;
            Result.Width = Codepoint.Width;
            Result.Offset = Offset;
            Result.Buffer = Iterator.Buffer;
            break;
        }
    }

    return Result;
}

internal inline b32
IsValid(text_iterator Iterator)
{
    b32 Result = (Iterator.Width > 0);
    return Result;
}

internal inline b32
IsDrawableCodepoint(u32 Codepoint)
{
    b32 Result = (FONT_ASCII_OFFSET <= Codepoint) && (Codepoint <= FONT_ASCII_OFFSET + FONT_ASCII_COUNT);
    return Result;
}

internal inline b32
IsValidAndNotNewLine(text_iterator Iterator)
{
    b32 Result = IsValid(Iterator) && (Iterator.Codepoint != '\n');
    return Result;
}

internal inline u32
GetCodepointColumnCount(editor_settings *Settings, u32 Codepoint, u32 ColumnOffset)
{
    u32 Result = 1;
    if (Codepoint == '\t')
    {
        Result = Settings->TabWidth - (ColumnOffset % Settings->TabWidth);
    }
    return Result;
}

internal inline memory_size
GetNumberOfLines(char *Base, memory_size Count)
{
    memory_size Result = 0;
    for (u32 Index = 0; Index < Count; ++Index)
    {
        if (Base[Index] == '\n')
        {
            ++Result;
        }
    }

    return Result;
}

internal inline u32
GetBufferOffsetOfLine(text_buffer Buffer, u32 Line)
{
    u32 Offset = 0;
    while (Line--)
    {
        while (Offset < Buffer.Used && Buffer.Base[Offset++] != '\n');
    }

    return Offset;
}

internal inline memory_size
GetBufferOffset(text_buffer Buffer, editor_settings *Settings, u32 Line, u32 Column)
{
    memory_size Result = GetBufferOffsetOfLine(Buffer, Line);
    text_iterator Iterator = NewTextIterator(&Buffer, Result);

    u32 ColumnOffset = 0;
    while (ColumnOffset < Column)
    {
        Assert(IsValidAndNotNewLine(Iterator));

        ColumnOffset += GetCodepointColumnCount(Settings, Iterator.Codepoint, ColumnOffset);
        Result += Iterator.Width;
        Iterator = AdvanceIterator(Iterator);
    }

    Assert(ColumnOffset == Column);
    return Result;
}

internal inline text_iterator
GetCurrentLineFirstIterator(text_iterator Iterator)
{
    text_iterator Result = DevanceIterator(Iterator);
    while (IsValidAndNotNewLine(Result))
    {
        Result = DevanceIterator(Result);
    }

    if (!IsValid(Result))
    {
        Result = NewTextIterator(Iterator.Buffer, 0);
    }
    else
    {
        Result = AdvanceIterator(Result);
    }

    return Result;
}

internal inline text_iterator
GetCurrentLineLastIterator(text_iterator Iterator)
{
    text_iterator Result = Iterator;
    while (IsValidAndNotNewLine(Result))
    {
        Result = AdvanceIterator(Result);
    }

    if (!IsValid(Iterator) && Iterator.Buffer->Used > 0)
    {
        Result = NewTextIterator(Iterator.Buffer, Iterator.Buffer->Used - 1);
    }

    return Result;
}

internal inline text_iterator
GetPreviousLineLastIterator(text_iterator Iterator)
{
    text_iterator Result = DevanceIterator(Iterator);
    while (IsValid(Result) && Result.Codepoint != '\n')
    {
        Result = DevanceIterator(Result);
    }

    return Result;
}

internal inline text_iterator
GetPreviousLineFirstIterator(text_iterator Iterator)
{
    text_iterator Result = GetPreviousLineLastIterator(Iterator);
    if (IsValid(Result))
    {
        Result = GetCurrentLineFirstIterator(Result);
    }

    return Result;
}

internal inline text_iterator
GetNextLineFirstIterator(text_iterator Iterator)
{
    text_iterator Result = Iterator;
    while (IsValid(Result) && Result.Codepoint != '\n')
    {
        Result = AdvanceIterator(Result);
    }

    if (IsValid(Result))
    {
        Result = AdvanceIterator(Result);
    }

    return Result;
}

internal inline text_iterator
GetLastCharacterIterator(text_buffer *Buffer)
{
    text_iterator Result = {};
    
    if (Buffer->Used > 0)
    {
        Result.Offset = Buffer->Used - 1;
        Result.Buffer = Buffer;
        Result = DevanceIterator(Result);
    }

    return Result;
}

internal inline u32
GetCodepointToggledCapital(u32 Codepoint)
{
    if ('A' <= Codepoint && Codepoint <= 'Z')
    {
        u32 Result = Codepoint + ('a' - 'A');
        return Result;
    }
    if ('a' <= Codepoint && Codepoint <= 'z')
    {
        u32 Result = Codepoint - ('a' - 'A');
        return Result;
    }

    return Codepoint;
}

internal inline u32
GetCodepointShiftCorrespondant(u32 Codepoint)
{
    u32 Result = GetCodepointToggledCapital(Codepoint);
    if (Result != Codepoint)
    {
        return Result;
    }

    switch (Codepoint)
    {
        case '0':  return ')';
        case '1':  return '!';
        case '2':  return '@';
        case '3':  return '#';
        case '4':  return '$';
        case '5':  return '%';
        case '6':  return '^';
        case '7':  return '&';
        case '8':  return '*';
        case '9':  return '(';

        case ';':  return ':';
        case ',':  return '<';
        case '.':  return '>';
        case '=':  return '+';
        case '-':  return '_';
        case '\'': return '"';
        case '`':  return '~';
        case '/':  return '?';
        case '\\': return '|';
        case '[':  return '{';
        case ']':  return '}';
    }

    return Result;
}

internal inline u32
GetCodepointFromKeyCode(key_code KeyCode)
{
    if (KeyCode_AlphabetKeyFirst <= KeyCode && KeyCode <= KeyCode_AlphabetKeyLast)
    {
        u32 Codepoint = 'a' + (KeyCode - KeyCode_AlphabetKeyFirst);
        return Codepoint;
    }
    if (KeyCode_Zero <= KeyCode && KeyCode <= KeyCode_Nine)
    {
        u32 Codepoint = '0' + (KeyCode - KeyCode_Zero);
        return Codepoint;
    }

    switch (KeyCode)
    {
        case KeyCode_Space:        return ' ';
        case KeyCode_Enter:        return '\n';
        case KeyCode_Tab:          return '\t';

        case KeyCode_Semicolon:    return ';';
        case KeyCode_Comma:        return ',';
        case KeyCode_Period:       return '.';
        case KeyCode_Equals:       return '=';
        case KeyCode_Minus:        return '-';
        case KeyCode_Apostrophe:   return '\'';
        case KeyCode_Backtick:     return '`';
        case KeyCode_Slash:        return '/';
        case KeyCode_Backslash:    return '\\';
        case KeyCode_LeftBracket:  return '[';
        case KeyCode_RightBracket: return ']';
    }

    InvalidCodePath;
    return 0;
}

#define OCEAN_TEXT_H
#endif // OCEAN_TEXT_H
