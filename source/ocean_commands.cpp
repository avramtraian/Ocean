/*  =====================================================================
    $File:   ocean_commands.cpp $
    $Date:   October 2 2023 $
    $Author: Traian Avram $
    $Notice: Copyright (c) 2023-2023 Traian Avram. All Rights Reserved. $
    =====================================================================  */

#include "ocean.h"
#include "ocean_math.h"
#include "ocean_text.h"

#if OCEAN_DEBUG
    #define VALIDATE_CARET_OFFSET(EditorState, PanelIndex)                                          \
        {                                                                                           \
            text_panel *Panel = EditorState->TextPanels + (PanelIndex);                             \
            memory_size ExpectedOffset = GetBufferOffset(Panel->Buffer, &EditorState->Settings,     \
                                                         Panel->Caret.Position.Line, Panel->Caret.Position.Column);   \
            Assert(Panel->Caret.Position.Offset == ExpectedOffset);                                          \
        }
#else
    #define VALIDATE_CARET_OFFSET(...)
#endif // OCEAN_DEBUG

#define EDITOR_COMMAND_CAST_DATA(CommandDataType)    \
            Assert(CommandInfo->OpaqueData != NULL); \
            CommandDataType *CommandData = (CommandDataType *)(CommandInfo->OpaqueData)

//=========================================================================================
// NOTE(traian): FILE NAVIGATION COMMANDS.
//=========================================================================================

internal EDITOR_COMMAND(Command_ScrollWindowToFitCaret)
{
    text_panel *Panel = EditorState->TextPanels + PanelIndex;
    text_caret *Caret = &Panel->Caret;

    if (Caret->Position.Line >= Panel->FirstLineIndex + Panel->ScreenLineCount)
    {
        Panel->FirstLineIndex = Caret->Position.Line - Panel->ScreenLineCount + 1;
        Panel->BufferOffset = GetBufferOffsetOfLine(Panel->Buffer, Panel->FirstLineIndex);
    }
    else if (Caret->Position.Line < Panel->FirstLineIndex)
    {
        Panel->FirstLineIndex = Caret->Position.Line;
        Panel->BufferOffset = GetBufferOffsetOfLine(Panel->Buffer, Panel->FirstLineIndex);
    }

    if (Caret->Position.Column >= Panel->FirstColumnIndex + Panel->ScreenColumnCount)
    {
        Panel->FirstColumnIndex = Caret->Position.Column - Panel->ScreenColumnCount;
    }
    else if (Caret->Position.Column < Panel->FirstColumnIndex)
    {
        Panel->FirstColumnIndex = Caret->Position.Column;
    }
}

internal EDITOR_COMMAND(Command_PageUp)
{
    text_panel *Panel = EditorState->TextPanels + PanelIndex;

    if (Panel->Caret.IsSelecting)
    {
        Panel->Caret.IsSelecting = false;
        Panel->Caret.Selection = {};
    }

    s32 RelativeLine = Panel->Caret.Position.Line - Panel->FirstLineIndex;
    if (RelativeLine < 0 || RelativeLine >= Panel->ScreenLineCount)
    {
        RelativeLine = Panel->ScreenLineCount / 2;
    }
    else if (Panel->FirstLineIndex == 0)
    {
        RelativeLine = 0;
    }

    if (Panel->FirstLineIndex >= Panel->ScreenLineCount)
    {
        Panel->FirstLineIndex -= Panel->ScreenLineCount;
    }
    else if (Panel->FirstLineIndex > 0)
    {
        Panel->FirstLineIndex = 0;
    }

    Panel->BufferOffset = GetBufferOffsetOfLine(Panel->Buffer, Panel->FirstLineIndex);
    Panel->Caret.Position.Line = Panel->FirstLineIndex + RelativeLine;
    Panel->Caret.Position.Column = 0;
    Panel->Caret.Position.Offset = GetBufferOffset(Panel->Buffer, &EditorState->Settings,
                                                   Panel->Caret.Position.Line, Panel->Caret.Position.Column);
    Panel->Caret.TargetColumn = Panel->Caret.Position.Column;

    Command_ScrollWindowToFitCaret(EditorState, PanelIndex, NULL);
    VALIDATE_CARET_OFFSET(EditorState, PanelIndex);
}

internal EDITOR_COMMAND(Command_PageDown)
{
    text_panel *Panel = EditorState->TextPanels + PanelIndex;

    if (Panel->Caret.IsSelecting)
    {
        Panel->Caret.IsSelecting = false;
        Panel->Caret.Selection = {};
    }

    s32 RelativeLine = Panel->Caret.Position.Line - Panel->FirstLineIndex;
    if (RelativeLine < 0 || RelativeLine >= Panel->ScreenLineCount)
    {
        RelativeLine = Panel->ScreenLineCount / 2;
    }
    else if (Panel->FirstLineIndex + Panel->ScreenLineCount >= Panel->LineCount)
    {
        RelativeLine = (Panel->LineCount - Panel->FirstLineIndex) - 1;
    }

    if (Panel->FirstLineIndex + Panel->ScreenLineCount < Panel->LineCount)
    {
        Panel->FirstLineIndex += Panel->ScreenLineCount;
    }

    Panel->BufferOffset = GetBufferOffsetOfLine(Panel->Buffer, Panel->FirstLineIndex);
    Panel->Caret.Position.Line = Panel->FirstLineIndex + RelativeLine;
    Panel->Caret.Position.Column = 0;
    Panel->Caret.Position.Offset = GetBufferOffset(Panel->Buffer, &EditorState->Settings,
                                                   Panel->Caret.Position.Line, Panel->Caret.Position.Column);
    Panel->Caret.TargetColumn = Panel->Caret.Position.Column;

    Command_ScrollWindowToFitCaret(EditorState, PanelIndex, NULL);
    VALIDATE_CARET_OFFSET(EditorState, PanelIndex);
}

internal EDITOR_COMMAND(Command_MoveCaretToLeft)
{
    editor_settings *Settings = &EditorState->Settings;
    text_panel *Panel = EditorState->TextPanels + PanelIndex;
    text_caret *Caret = &Panel->Caret;

    if (Panel->Caret.Position.Offset > 0)
    {
        text_iterator Iterator = NewTextIterator(&Panel->Buffer, Panel->Caret.Position.Offset);
        
        if (Panel->Caret.Position.Column == 0)
        {
            Iterator = GetPreviousLineFirstIterator(Iterator);
            u32 ColumnOffset = 0;
            while (Iterator.Codepoint != '\n')
            {
                ColumnOffset += GetCodepointColumnCount(Settings, Iterator.Codepoint, ColumnOffset);
                Iterator = AdvanceIterator(Iterator);
            }

            Panel->Caret.Position.Line--;
            Panel->Caret.Position.Column = ColumnOffset;
            Panel->Caret.Position.Offset = Iterator.Offset;
        }
        else
        {
            text_iterator PreviousIterator = DevanceIterator(Iterator);
            memory_offset TargetBufferOffset = Iterator.Offset - PreviousIterator.Width;
            Iterator = GetCurrentLineFirstIterator(Iterator);

            u32 ColumnOffset = 0;
            memory_offset BufferOffset = Iterator.Offset;
            while (BufferOffset < TargetBufferOffset)
            {
                ColumnOffset += GetCodepointColumnCount(Settings, Iterator.Codepoint, ColumnOffset);
                BufferOffset += Iterator.Width;
                Iterator = AdvanceIterator(Iterator);
            }

            Assert(BufferOffset == TargetBufferOffset);
            Panel->Caret.Position.Column = ColumnOffset;
            Panel->Caret.Position.Offset = TargetBufferOffset;
        }

        Panel->Caret.TargetColumn = Panel->Caret.Position.Column;
    }

    Command_ScrollWindowToFitCaret(EditorState, PanelIndex, NULL);
    VALIDATE_CARET_OFFSET(EditorState, PanelIndex);
}

internal EDITOR_COMMAND(Command_MoveCaretToRight)
{
    editor_settings *Settings = &EditorState->Settings;
    text_panel *Panel = EditorState->TextPanels + PanelIndex;
    text_caret *Caret = &Panel->Caret;

    if (Panel->Caret.Position.Offset < Panel->Buffer.Used)
    {
        text_iterator Iterator = NewTextIterator(&Panel->Buffer, Panel->Caret.Position.Offset);
        if (Iterator.Codepoint == '\n')
        {
            Panel->Caret.Position.Line++;
            Panel->Caret.Position.Column = 0;
        }
        else
        {
            Panel->Caret.Position.Column += GetCodepointColumnCount(Settings, Iterator.Codepoint, Panel->Caret.Position.Column);
        }

        Panel->Caret.Position.Offset += Iterator.Width;
        Panel->Caret.TargetColumn = Panel->Caret.Position.Column;
    }

    Command_ScrollWindowToFitCaret(EditorState, PanelIndex, NULL);
    VALIDATE_CARET_OFFSET(EditorState, PanelIndex);
}

internal EDITOR_COMMAND(Command_MoveCaretUp)
{
    editor_settings *Settings = &EditorState->Settings;
    text_panel *Panel = EditorState->TextPanels + PanelIndex;
    text_caret *Caret = &Panel->Caret;

    if (Panel->Caret.Position.Line > 0)
    {
        text_iterator Iterator = NewTextIterator(&Panel->Buffer, Caret->Position.Offset);
        Iterator = GetPreviousLineFirstIterator(Iterator);
        Assert(IsValid(Iterator));

        u32 ColumnOffset = 0;
        while (ColumnOffset < Caret->TargetColumn && IsValidAndNotNewLine(Iterator))
        {
            ColumnOffset += GetCodepointColumnCount(Settings, Iterator.Codepoint, ColumnOffset);
            Iterator = AdvanceIterator(Iterator);
        }

        Caret->Position.Line--;
        Caret->Position.Column = ColumnOffset;
        Assert(IsValid(Iterator));
        Caret->Position.Offset = Iterator.Offset;
    }

    Command_ScrollWindowToFitCaret(EditorState, PanelIndex, NULL);
    VALIDATE_CARET_OFFSET(EditorState, PanelIndex);
}

internal EDITOR_COMMAND(Command_MoveCaretDown)
{
    editor_settings *Settings = &EditorState->Settings;
    text_panel *Panel = EditorState->TextPanels + PanelIndex;
    text_caret *Caret = &Panel->Caret;

    if (Caret->Position.Line < Panel->LineCount)
    {
        text_iterator Iterator = NewTextIterator(&Panel->Buffer, Caret->Position.Offset);
        Iterator = GetNextLineFirstIterator(Iterator);

        if (IsValid(Iterator))
        {
            u32 ColumnOffset = 0;
            while (ColumnOffset < Caret->TargetColumn && IsValidAndNotNewLine(Iterator))
            {
                ColumnOffset += GetCodepointColumnCount(Settings, Iterator.Codepoint, ColumnOffset);
                Iterator = AdvanceIterator(Iterator);
            }

            Caret->Position.Column = ColumnOffset;
            Caret->Position.Offset = IsValid(Iterator) ? Iterator.Offset : Panel->Buffer.Used;
        }
        else
        {
            Caret->Position.Column = 0;
            Caret->Position.Offset = Panel->Buffer.Used;
        }

        Caret->Position.Line++;
    }

    Command_ScrollWindowToFitCaret(EditorState, PanelIndex, NULL);
    VALIDATE_CARET_OFFSET(EditorState, PanelIndex);
}

internal EDITOR_COMMAND(Command_ArrowLeft)
{
    text_panel *Panel = EditorState->TextPanels + PanelIndex;

    if (Panel->Caret.IsSelecting)
    {
        text_caret_position Position = Panel->Caret.Position;
        if (Panel->Caret.Selection.Offset < Position.Offset)
        {
            Position = Panel->Caret.Selection;
        }

        Panel->Caret.IsSelecting = false;
        Panel->Caret.Selection = {};
        Panel->Caret.Position = Position;

        Command_ScrollWindowToFitCaret(EditorState, PanelIndex, NULL);
        return;
    }

    Command_MoveCaretToLeft(EditorState, PanelIndex, NULL);
}

internal EDITOR_COMMAND(Command_ArrowRight)
{
    text_panel *Panel = EditorState->TextPanels + PanelIndex;

    if (Panel->Caret.IsSelecting)
    {
        text_caret_position Position = Panel->Caret.Position;
        if (Panel->Caret.Selection.Offset > Position.Offset)
        {
            Position = Panel->Caret.Selection;
        }

        Panel->Caret.IsSelecting = false;
        Panel->Caret.Selection = {};
        Panel->Caret.Position = Position;
        
        Command_ScrollWindowToFitCaret(EditorState, PanelIndex, NULL);
        return;
    }

    Command_MoveCaretToRight(EditorState, PanelIndex, NULL);
}

internal EDITOR_COMMAND(Command_ArrowUp)
{
    text_panel *Panel = EditorState->TextPanels + PanelIndex;

    if (Panel->Caret.IsSelecting)
    {
        Panel->Caret.IsSelecting = false;
        Panel->Caret.Selection = {};
    }

    Command_MoveCaretUp(EditorState, PanelIndex, NULL);
}

internal EDITOR_COMMAND(Command_ArrowDown)
{
    text_panel *Panel = EditorState->TextPanels + PanelIndex;

    if (Panel->Caret.IsSelecting)
    {
        Panel->Caret.IsSelecting = false;
        Panel->Caret.Selection = {};
    }

    Command_MoveCaretDown(EditorState, PanelIndex, NULL);
}

internal inline b32
IsContiguousTokenCodepoint(u32 Codepoint)
{
    b32 Result = ('a' <= Codepoint && Codepoint <= 'z') ||
                 ('A' <= Codepoint && Codepoint <= 'Z') ||
                 (Codepoint == '_');
    return Result;
}

internal inline b32
IsIgnoredTokenCodepoint(u32 Codepoint)
{
    b32 Result = (Codepoint == ' ' || Codepoint == '\t');
    return Result;
}

internal EDITOR_COMMAND(Command_GoToNextToken)
{
    text_panel *Panel = EditorState->TextPanels + PanelIndex;
    text_caret *Caret = &Panel->Caret;

    if (Caret->Position.Offset < Panel->Buffer.Used)
    {
        text_iterator Iterator = NewTextIterator(&Panel->Buffer, Caret->Position.Offset);

        if (IsContiguousTokenCodepoint(Iterator.Codepoint))
        {
            while (IsValid(Iterator) && IsContiguousTokenCodepoint(Iterator.Codepoint))
            {
                Command_MoveCaretToRight(EditorState, PanelIndex, NULL);
                Iterator = AdvanceIterator(Iterator);
            }
            while (IsValid(Iterator) && IsIgnoredTokenCodepoint(Iterator.Codepoint))
            {
                Command_MoveCaretToRight(EditorState, PanelIndex, NULL);
                Iterator = AdvanceIterator(Iterator);
            }
        }
        else if (IsIgnoredTokenCodepoint(Iterator.Codepoint))
        {
            while (IsValid(Iterator) && IsIgnoredTokenCodepoint(Iterator.Codepoint))
            {
                Command_MoveCaretToRight(EditorState, PanelIndex, NULL);
                Iterator = AdvanceIterator(Iterator);
            }
        }
        else
        {
            Command_MoveCaretToRight(EditorState, PanelIndex, NULL);
        }
    }

    VALIDATE_CARET_OFFSET(EditorState, PanelIndex);
    Command_ScrollWindowToFitCaret(EditorState, PanelIndex, NULL);
}

internal EDITOR_COMMAND(Command_GoToPreviousToken)
{
    text_panel *Panel = EditorState->TextPanels + PanelIndex;
    text_caret *Caret = &Panel->Caret;

    if (Caret->Position.Offset > 0)
    {
        text_iterator Iterator = NewTextIterator(&Panel->Buffer, Caret->Position.Offset);
        Iterator = DevanceIterator(Iterator);

        if (IsContiguousTokenCodepoint(Iterator.Codepoint))
        {
            while (IsValid(Iterator) && IsContiguousTokenCodepoint(Iterator.Codepoint))
            {
                Command_MoveCaretToLeft(EditorState, PanelIndex, NULL);
                Iterator = DevanceIterator(Iterator);
            }

            text_caret CurrentCaret = *Caret;

            while (IsValid(Iterator) && IsIgnoredTokenCodepoint(Iterator.Codepoint))
            {
                Command_MoveCaretToLeft(EditorState, PanelIndex, NULL);
                Iterator = DevanceIterator(Iterator);
            }

            if (Iterator.Codepoint == '\n')
            {
                *Caret = CurrentCaret;
                Command_ScrollWindowToFitCaret(EditorState, PanelIndex, NULL);
            }
        }
        else if (IsIgnoredTokenCodepoint(Iterator.Codepoint))
        {
            while (IsValid(Iterator) && IsIgnoredTokenCodepoint(Iterator.Codepoint))
            {
                Command_MoveCaretToLeft(EditorState, PanelIndex, NULL);
                Iterator = DevanceIterator(Iterator);
            }
        }
        else
        {
            Command_MoveCaretToLeft(EditorState, PanelIndex, NULL);
        }
    }
}

internal EDITOR_COMMAND(Command_ScrollPanelUp)
{
    text_panel *Panel = EditorState->TextPanels + PanelIndex;

    if (Panel->FirstLineIndex > 0)
    {
        text_iterator Iterator = NewTextIterator(&Panel->Buffer, Panel->BufferOffset);
        Iterator = GetPreviousLineFirstIterator(Iterator);
        Assert(IsValid(Iterator));

        Panel->BufferOffset = Iterator.Offset;
        Panel->FirstLineIndex--;
    }
}

internal EDITOR_COMMAND(Command_ScrollPanelDown)
{
    text_panel *Panel = EditorState->TextPanels + PanelIndex;

    if (Panel->FirstLineIndex < Panel->LineCount - 1)
    {
        text_iterator Iterator = NewTextIterator(&Panel->Buffer, Panel->BufferOffset);
        Iterator = GetNextLineFirstIterator(Iterator);
        Assert(IsValid(Iterator));

        Panel->BufferOffset = Iterator.Offset;
        Panel->FirstLineIndex++;
    }
}

//=========================================================================================
// NOTE(traian): SELECTION MANIPULATION.
//=========================================================================================

internal EDITOR_COMMAND(Command_SelectArrowLeft)
{
    text_panel *Panel = EditorState->TextPanels + PanelIndex;
    text_caret *Caret = &Panel->Caret;

    if (Caret->IsSelecting)
    {
        Command_MoveCaretToLeft(EditorState, PanelIndex, NULL);
        if (Caret->Position.Offset == Caret->Selection.Offset)
        {
            Caret->IsSelecting = false;
            Caret->Selection = {};
        }
    }
    else
    {
        Caret->IsSelecting = true;
        Caret->Selection = Caret->Position;
        Command_MoveCaretToLeft(EditorState, PanelIndex, NULL);
    }
}

internal EDITOR_COMMAND(Command_SelectArrowRight)
{
    text_panel *Panel = EditorState->TextPanels + PanelIndex;
    text_caret *Caret = &Panel->Caret;

    if (Caret->IsSelecting)
    {
        Command_MoveCaretToRight(EditorState, PanelIndex, NULL);
        if (Caret->Position.Offset == Caret->Selection.Offset)
        {
            Caret->IsSelecting = false;
            Caret->Selection = {};
        }
    }
    else
    {
        Caret->Selection = Caret->Position;
        Command_MoveCaretToRight(EditorState, PanelIndex, NULL);
        Caret->IsSelecting = true;
    }
}

internal EDITOR_COMMAND(Command_SelectArrowUp)
{
    text_panel *Panel = EditorState->TextPanels + PanelIndex;
    text_caret *Caret = &Panel->Caret;

    if (Caret->IsSelecting)
    {
        Command_MoveCaretUp(EditorState, PanelIndex, NULL);
        if (Caret->Position.Offset == Caret->Selection.Offset)
        {
            Caret->IsSelecting = false;
            Caret->Selection = {};
        }
    }
    else
    {
        Caret->Selection = Caret->Position;
        Command_MoveCaretUp(EditorState, PanelIndex, NULL);
        Caret->IsSelecting = true;
    }
}

internal EDITOR_COMMAND(Command_SelectArrowDown)
{
    text_panel* Panel = EditorState->TextPanels + PanelIndex;
    text_caret* Caret = &Panel->Caret;

    if (Caret->IsSelecting)
    {
        Command_MoveCaretDown(EditorState, PanelIndex, NULL);
        if (Caret->Position.Offset == Caret->Selection.Offset)
        {
            Caret->IsSelecting = false;
            Caret->Selection = {};
        }
    }
    else
    {
        Caret->Selection = Caret->Position;
        Command_MoveCaretDown(EditorState, PanelIndex, NULL);
        Caret->IsSelecting = true;
    }
}

internal EDITOR_COMMAND(Command_SelectUntilNextToken)
{
    text_panel* Panel = EditorState->TextPanels + PanelIndex;
    text_caret* Caret = &Panel->Caret;

    if (Caret->IsSelecting)
    {
        Command_GoToNextToken(EditorState, PanelIndex, NULL);
        if (Caret->Position.Offset == Caret->Selection.Offset)
        {
            Caret->IsSelecting = false;
            Caret->Selection = {};
        }
    }
    else
    {
        Caret->Selection = Caret->Position;
        Command_GoToNextToken(EditorState, PanelIndex, NULL);
        Caret->IsSelecting = true;
    }
}

internal EDITOR_COMMAND(Command_SelectUntilPreviousToken)
{
    text_panel *Panel = EditorState->TextPanels + PanelIndex;
    text_caret *Caret = &Panel->Caret;

    if (Caret->IsSelecting)
    {
        Command_GoToPreviousToken(EditorState, PanelIndex, NULL);
        if (Caret->Position.Offset == Caret->Selection.Offset)
        {
            Caret->IsSelecting = false;
            Caret->Selection = {};
        }
    }
    else
    {
        Caret->Selection = Caret->Position;
        Command_GoToPreviousToken(EditorState, PanelIndex, NULL);
        Caret->IsSelecting = true;
    }
}

//=========================================================================================
// NOTE(traian): TEXT INPUT AND DELETION COMMANDS.
//=========================================================================================

struct command_remove_characters_data
{
    memory_size ByteCount;
};

internal EDITOR_COMMAND(Command_RemoveCharacters)
{
    EDITOR_COMMAND_CAST_DATA(command_remove_characters_data);

    text_panel *Panel = EditorState->TextPanels + PanelIndex;
    text_buffer *Buffer = &Panel->Buffer;

    if (Panel->Caret.Position.Offset + CommandData->ByteCount <= Buffer->Used && CommandData->ByteCount > 0)
    {
        text_iterator Iterator = NewTextIterator(Buffer, Panel->Caret.Position.Offset);

        memory_offset TargetOffset = Panel->Caret.Position.Offset + CommandData->ByteCount;
        while (IsValid(Iterator) && Iterator.Offset < TargetOffset)
        {
            if (Iterator.Codepoint == '\n')
            {
                Panel->LineCount--;
            }
            Iterator = AdvanceIterator(Iterator);
        }

        Buffer->Used -= CommandData->ByteCount;
        for (memory_offset BufferOffset = Panel->Caret.Position.Offset; BufferOffset < Buffer->Used; ++BufferOffset)
        {
            Buffer->Base[BufferOffset] = Buffer->Base[BufferOffset + CommandData->ByteCount];
        }

        Panel->IsSaveDirty = true;
    }

    Command_ScrollWindowToFitCaret(EditorState, PanelIndex, NULL);
}

EDITOR_COMMAND(Command_RemoveCharacterFromRight)
{
   text_panel *Panel = EditorState->TextPanels + PanelIndex;

    if (Panel->Caret.IsSelecting)
    {
        text_caret_position Position = Panel->Caret.Selection;
        memory_size RemoveByteCount = Panel->Caret.Position.Offset - Panel->Caret.Selection.Offset;
        if (Panel->Caret.Position.Offset < Panel->Caret.Selection.Offset)
        {
            Position = Panel->Caret.Position;
            RemoveByteCount = Panel->Caret.Selection.Offset - Panel->Caret.Position.Offset;
        }

        Panel->Caret.IsSelecting = false;
        Panel->Caret.Position = Position;
        Panel->Caret.Selection = {};

        command_remove_characters_data RemoveCommandData = {};
        RemoveCommandData.ByteCount = RemoveByteCount;
        editor_command_info RemoveCommandInfo = {};
        RemoveCommandInfo.OpaqueData = &RemoveCommandData;

        Command_RemoveCharacters(EditorState, PanelIndex, &RemoveCommandInfo);
    }
    else if (Panel->Caret.Position.Offset < Panel->Buffer.Used)
    {
        text_iterator Iterator = NewTextIterator(&Panel->Buffer, Panel->Caret.Position.Offset);
        
        command_remove_characters_data RemoveCommandData = {};
        RemoveCommandData.ByteCount = Iterator.Width;
        editor_command_info RemoveCommandInfo = {};
        RemoveCommandInfo.OpaqueData = &RemoveCommandData;

        Command_RemoveCharacters(EditorState, PanelIndex, &RemoveCommandInfo);
    }
}

internal EDITOR_COMMAND(Command_RemoveCharacterFromLeft)
{
    text_panel *Panel = EditorState->TextPanels + PanelIndex;

    if (Panel->Caret.IsSelecting)
    {
        text_caret_position Position = Panel->Caret.Selection;
        memory_size RemoveByteCount = Panel->Caret.Position.Offset - Panel->Caret.Selection.Offset;
        if (Panel->Caret.Position.Offset < Panel->Caret.Selection.Offset)
        {
            Position = Panel->Caret.Position;
            RemoveByteCount = Panel->Caret.Selection.Offset - Panel->Caret.Position.Offset;
        }

        Panel->Caret.IsSelecting = false;
        Panel->Caret.Position = Position;
        Panel->Caret.Selection = {};

        command_remove_characters_data RemoveCommandData = {};
        RemoveCommandData.ByteCount = RemoveByteCount;
        editor_command_info CommandInfo = {};
        CommandInfo.OpaqueData = &RemoveCommandData;

        Command_RemoveCharacters(EditorState, PanelIndex, &CommandInfo);
    }
    else if (Panel->Caret.Position.Offset > 0)
    {
        Command_MoveCaretToLeft(EditorState, PanelIndex, NULL);
        Command_RemoveCharacterFromRight(EditorState, PanelIndex, NULL);
    }
}

struct command_insert_characters_data
{
    char *Characters;
    memory_size ByteCount;
};

internal EDITOR_COMMAND(Command_InsertCharacters)
{
    EDITOR_COMMAND_CAST_DATA(command_insert_characters_data);

    text_panel *Panel = EditorState->TextPanels + PanelIndex;
    text_caret *Caret = &Panel->Caret;
    text_buffer *Buffer = &Panel->Buffer;

    text_buffer InsertBuffer = {};
    InsertBuffer.Base = CommandData->Characters;
    InsertBuffer.Size = CommandData->ByteCount;
    InsertBuffer.Used = CommandData->ByteCount;

    if (Buffer->Used + CommandData->ByteCount > Buffer->Size)
    {
        memory_size NewTextBufferSize = Maximum(Buffer->Used + CommandData->ByteCount, 2 * Buffer->Size);
        buffer OldTextBuffer;
        OldTextBuffer.Data = (u8 *)Buffer->Base;
        OldTextBuffer.Size = Buffer->Size;

        buffer NewTextBuffer = PlatformAllocateMemory(NewTextBufferSize);
        Buffer->Base = (char *)NewTextBuffer.Data;
        Buffer->Size = NewTextBuffer.Size;
        
        if (OldTextBuffer.Data)
        {
            CopyMem(Buffer->Base, OldTextBuffer.Data, Buffer->Used);
            PlatformReleaseMemory(OldTextBuffer);
        }
    }

    Buffer->Used += CommandData->ByteCount;
    memory_offset InsertOffset = Caret->Position.Offset;

    for (memory_offset BufferOffset = Buffer->Used - 1;
        BufferOffset >= InsertOffset + CommandData->ByteCount;
        --BufferOffset)
    {
        Buffer->Base[BufferOffset] = Buffer->Base[BufferOffset - CommandData->ByteCount];
    }

    CopyMem(Buffer->Base + InsertOffset, CommandData->Characters, CommandData->ByteCount);
    Panel->IsSaveDirty = true;

    text_iterator Iterator = NewTextIterator(&InsertBuffer, 0);
    while (IsValid(Iterator))
    {
        if (Iterator.Codepoint == '\n')
        {
            Caret->Position.Line++;
            Caret->Position.Column = 0;
            Panel->LineCount++;
        }
        else
        {
            Caret->Position.Column += GetCodepointColumnCount(&EditorState->Settings, Iterator.Codepoint, Caret->Position.Column);
        }

        Caret->Position.Offset += Iterator.Width;
        Iterator = AdvanceIterator(Iterator);
    }

    Command_ScrollWindowToFitCaret(EditorState, PanelIndex, NULL);
    VALIDATE_CARET_OFFSET(EditorState, PanelIndex);
}

internal EDITOR_COMMAND(Command_InsertCharacter)
{
    text_panel *Panel = EditorState->TextPanels + PanelIndex;

    if (Panel->Caret.IsSelecting)
    {
        text_caret_position Position = Panel->Caret.Selection;
        memory_size RemoveByteCount = Panel->Caret.Position.Offset - Panel->Caret.Selection.Offset;
        if (Panel->Caret.Position.Offset < Panel->Caret.Selection.Offset)
        {
            Position = Panel->Caret.Position;
            RemoveByteCount = Panel->Caret.Selection.Offset - Panel->Caret.Position.Offset;
        }

        Panel->Caret.IsSelecting = false;
        Panel->Caret.Position = Position;
        Panel->Caret.Selection = {};

        command_remove_characters_data RemoveCommandData = {};
        RemoveCommandData.ByteCount = RemoveByteCount;
        editor_command_info CommandInfo = {};
        CommandInfo.OpaqueData = &RemoveCommandData;

        Command_RemoveCharacters(EditorState, PanelIndex, &CommandInfo);
    }
    
    u32 Codepoint = GetCodepointFromKeyCode(CommandInfo->KeyCode);
    if (CommandInfo->Modifiers & KeyModifier_Shift)
    {
        Codepoint = GetCodepointShiftCorrespondant(Codepoint);
    }
    if (CommandInfo->IsCapsLockActive)
    {
        Codepoint = GetCodepointToggledCapital(Codepoint);
    }

    // TODO(traian): Unicode encodings!
    char Buffer[4] = { (char)Codepoint };
    u32 ByteCount = 1;

    if (Codepoint == '\t' && EditorState->Settings.ReplaceTabWithSpaces)
    {
        ByteCount = GetCodepointColumnCount(&EditorState->Settings, '\t', Panel->Caret.Position.Column);
        SetMemory(Buffer, ' ', ByteCount);
    }

    command_insert_characters_data InsertCharactersData = {};
    InsertCharactersData.Characters = Buffer;
    InsertCharactersData.ByteCount = ByteCount;
    editor_command_info InsertCommandInfo = {};
    InsertCommandInfo.OpaqueData = &InsertCharactersData;

    Command_InsertCharacters(EditorState, PanelIndex, &InsertCommandInfo);
}

internal EDITOR_COMMAND(Command_RemoveCharactersUntilNextToken)
{
    text_panel *Panel = EditorState->TextPanels + PanelIndex;

    text_caret CurrentCaret = Panel->Caret;
    Command_GoToNextToken(EditorState, PanelIndex, NULL);
    memory_size BytesToRemove = Panel->Caret.Position.Offset - CurrentCaret.Position.Offset;
    Panel->Caret = CurrentCaret;

    command_remove_characters_data CommandData = {};
    CommandData.ByteCount = BytesToRemove;
    editor_command_info RemoveCommandInfo = {};
    RemoveCommandInfo.OpaqueData = &CommandData;

    Command_RemoveCharacters(EditorState, PanelIndex, &RemoveCommandInfo);
}

internal EDITOR_COMMAND(Command_RemoveCharactersUntilPreviousToken)
{
    text_panel *Panel = EditorState->TextPanels + PanelIndex;
    
    memory_offset CurrentOffset = Panel->Caret.Position.Offset;
    Command_GoToPreviousToken(EditorState, PanelIndex, NULL);
    memory_size BytesToRemove = CurrentOffset - Panel->Caret.Position.Offset;

    command_remove_characters_data CommandData = {};
    CommandData.ByteCount = BytesToRemove;
    editor_command_info RemoveCommandInfo = {};
    RemoveCommandInfo.OpaqueData = &CommandData;

    Command_RemoveCharacters(EditorState, PanelIndex, &RemoveCommandInfo);
}

//=========================================================================================
// NOTE(traian): FILE MANAGEMENT COMMANDS.
//=========================================================================================

internal EDITOR_COMMAND(Command_SaveFile)
{
    text_panel *Panel = EditorState->TextPanels + PanelIndex;
    text_buffer *Buffer = &Panel->Buffer;

    if (Panel->FileName)
    {
        buffer TextBuffer;
        TextBuffer.Data = (u8 *)Buffer->Base;
        TextBuffer.Size = Buffer->Used;

        memory_size BytesWritten = PlatformWriteEntireFile(Panel->FileName, TextBuffer);
        Assert(BytesWritten == Buffer->Used);
        Panel->IsSaveDirty = false;
    }
}

internal inline void
ResetCaret(text_caret *Caret)
{
    Caret->Position.Offset = 0;
    Caret->Position.Line = 0;
    Caret->Position.Column = 0;
    Caret->TargetColumn = 0;
    Caret->IsSelecting = false;
}

struct command_open_file_data
{
    char *FileName;
};

internal EDITOR_COMMAND(Command_OpenFile)
{
    EDITOR_COMMAND_CAST_DATA(command_open_file_data);

    text_panel *Panel = EditorState->TextPanels + PanelIndex;
    text_buffer *Buffer = &Panel->Buffer;

    Panel->IsSaveDirty = false;
    Panel->FileName = NULL;
    Panel->LineCount = 0;

    ResetCaret(&Panel->Caret);
    Panel->FirstLineIndex = 0;
    Panel->FirstColumnIndex = 0;
    Panel->BufferOffset = 0;

    memory_size FileSize = PlatformGetFileSize(CommandData->FileName);
    if (FileSize != INVALID_SIZE)
    {
        memory_size TextBufferSize = FileSize + FileSize / 2;
        if (TextBufferSize > Buffer->Size || Buffer->Size > (2 * TextBufferSize))
        {
            buffer OldTextBuffer = { (u8 *)Buffer->Base, Buffer->Size };
            PlatformReleaseMemory(OldTextBuffer);

            buffer NewTextBuffer = PlatformAllocateMemory(TextBufferSize);
            Buffer->Size = TextBufferSize;
            Buffer->Base = (char *)NewTextBuffer.Data;
        }

        Buffer->Used = FileSize;

        buffer FileBuffer = { (u8 *)Buffer->Base, Buffer->Size };
        memory_size ReadByteCount = PlatformReadEntireFile(CommandData->FileName, FileBuffer);
        Assert(ReadByteCount == Buffer->Used);

        Panel->FileName = CommandData->FileName;
        Panel->LineCount = GetNumberOfLines(Buffer->Base, Buffer->Used);
    }
}

internal EDITOR_COMMAND(Command_NewTextBuffer)
{
    text_panel *Panel = EditorState->TextPanels + PanelIndex;
    text_buffer *Buffer = &Panel->Buffer;

    Panel->IsSaveDirty = false;
    Panel->FileName = NULL;
    Panel->LineCount = 0;
    
    ResetCaret(&Panel->Caret);
    Panel->FirstLineIndex = 0;
    Panel->FirstColumnIndex = 0;
    Panel->BufferOffset = 0;

    if (Buffer->Size == 0)
    {
        buffer TextBuffer = PlatformAllocateMemory(Kilobytes(2));
        Buffer->Base = (char *)TextBuffer.Data;
        Buffer->Size = TextBuffer.Size;
    }

    SetMemoryToZero(Buffer->Base, Buffer->Size);
    Buffer->Used = 0;
}

//=========================================================================================
// NOTE(traian): EDITOR MANAGEMENT.
//=========================================================================================

internal EDITOR_COMMAND(Command_Quit)
{
    PlatformQuit();
}

internal EDITOR_COMMAND(Command_ToggleFullscreen)
{
    PlatformToggleFullscreen();
}

internal EDITOR_COMMAND(Command_FocusOnTextPanelOne)
{
    EditorState->FocusedTextPanelIndex = 0;
}

internal EDITOR_COMMAND(Command_FocusOnTextPanelTwo)
{
    Assert(EditorState->EditorLayout >= EditorLayout_Dual);
    EditorState->FocusedTextPanelIndex = 1;
}

internal EDITOR_COMMAND(Command_FocusOnTextPanelThree)
{
    Assert(EditorState->EditorLayout >= EditorLayout_TripleLeft);
    EditorState->FocusedTextPanelIndex = 2;
}

internal EDITOR_COMMAND(Command_FocusOnTextPanelFour)
{
    Assert(EditorState->EditorLayout == EditorLayout_Quad);
    EditorState->FocusedTextPanelIndex = 3;
}

internal EDITOR_COMMAND(Command_ToggleFocusedTextPanel)
{
    u32 PanelCount = 1;
    switch (EditorState->EditorLayout)
    {
        case EditorLayout_Single:      { PanelCount = 1; } break;
        case EditorLayout_Dual:        { PanelCount = 2; } break;
        case EditorLayout_TripleLeft:  { PanelCount = 3; } break;
        case EditorLayout_TripleRight: { PanelCount = 3; } break;
        case EditorLayout_Quad:        { PanelCount = 4; } break;
    }

    EditorState->FocusedTextPanelIndex = (EditorState->FocusedTextPanelIndex + 1) % PanelCount;
}

//=========================================================================================
// NOTE(traian): BINDING AND EXECUTION OF EDITOR COMMANDS.
//=========================================================================================

internal inline u32
AllocateCommandEntry(command_table *CommandTable)
{
    Assert(CommandTable->CommandEntriesPoolOffset < ArrayCount(CommandTable->CommandEntriesPool))
    u32 EntryIndex = CommandTable->CommandEntriesPoolOffset++;
    CommandTable->CommandEntriesPool[EntryIndex].NextIndex = UINT32_MAX;
    return EntryIndex;
}

internal inline void
BindKeyCommand(command_table *CommandTable, u8 KeyCode, u8 Modifiers, editor_command_function *Callback)
{
    command_entry *Entry;
    u32 EntryIndex = CommandTable->KeyCommands[KeyCode];
    if (EntryIndex != UINT32_MAX)
    {
        u32 LastEntryIndex;
        while (EntryIndex != UINT32_MAX)
        {
            LastEntryIndex = EntryIndex;
            EntryIndex = CommandTable->CommandEntriesPool[EntryIndex].NextIndex;
        }

        EntryIndex = AllocateCommandEntry(CommandTable);
        Entry = CommandTable->CommandEntriesPool + EntryIndex;
        CommandTable->CommandEntriesPool[LastEntryIndex].NextIndex = EntryIndex;
    }
    else
    {
        EntryIndex = AllocateCommandEntry(CommandTable);
        Entry = CommandTable->CommandEntriesPool + EntryIndex;
        CommandTable->KeyCommands[KeyCode] = EntryIndex;
    }

    Entry->NextIndex = UINT32_MAX;
    Entry->Modifiers = (key_modifier)Modifiers;
    Entry->Callback = Callback;
}

void
InitializeEditorCommandTable(editor_state *EditorState)
{
    command_table *CommandTable = &EditorState->CommandTable;

    for (u16 CommandIndex = 0; CommandIndex < ArrayCount(CommandTable->KeyCommands); ++CommandIndex)
    {
        CommandTable->KeyCommands[CommandIndex] = UINT32_MAX;
    }

    //
    // NOTE(traian): Text input.
    //

    for (u8 KeyCode = KeyCode_AlphabetKeyFirst; KeyCode <= KeyCode_AlphabetKeyLast; ++KeyCode)
    {
        BindKeyCommand(CommandTable, KeyCode,  KeyModifier_None,  Command_InsertCharacter);
        BindKeyCommand(CommandTable, KeyCode,  KeyModifier_Shift, Command_InsertCharacter);
    }
    for (u8 KeyCode = KeyCode_Zero; KeyCode <= KeyCode_Nine; ++KeyCode)
    {
        BindKeyCommand(CommandTable, KeyCode,  KeyModifier_None,  Command_InsertCharacter);
        BindKeyCommand(CommandTable, KeyCode,  KeyModifier_Shift, Command_InsertCharacter);
    }
    for (u8 KeyCode = KeyCode_Semicolon; KeyCode <= KeyCode_RightBracket; ++KeyCode)
    {
        BindKeyCommand(CommandTable, KeyCode,  KeyModifier_None,  Command_InsertCharacter);
        BindKeyCommand(CommandTable, KeyCode,  KeyModifier_Shift, Command_InsertCharacter);
    }

    BindKeyCommand(CommandTable, KeyCode_Space,      KeyModifier_None,  Command_InsertCharacter);
    BindKeyCommand(CommandTable, KeyCode_Space,      KeyModifier_Shift, Command_InsertCharacter);
    BindKeyCommand(CommandTable, KeyCode_Enter,      KeyModifier_None,  Command_InsertCharacter);
    BindKeyCommand(CommandTable, KeyCode_Enter,      KeyModifier_Shift, Command_InsertCharacter);
    BindKeyCommand(CommandTable, KeyCode_Tab,        KeyModifier_None,  Command_InsertCharacter);
    BindKeyCommand(CommandTable, KeyCode_Tab,        KeyModifier_Shift, Command_InsertCharacter);
    
    //
    // NOTE(traian): File navigation.
    //

    BindKeyCommand(CommandTable, KeyCode_ArrowLeft,  KeyModifier_None,  Command_ArrowLeft);
    BindKeyCommand(CommandTable, KeyCode_ArrowRight, KeyModifier_None,  Command_ArrowRight);
    BindKeyCommand(CommandTable, KeyCode_ArrowUp,    KeyModifier_None,  Command_ArrowUp);
    BindKeyCommand(CommandTable, KeyCode_ArrowDown,  KeyModifier_None,  Command_ArrowDown);

    BindKeyCommand(CommandTable, KeyCode_ArrowLeft,  KeyModifier_Shift, Command_SelectArrowLeft);
    BindKeyCommand(CommandTable, KeyCode_ArrowRight, KeyModifier_Shift, Command_SelectArrowRight);
    BindKeyCommand(CommandTable, KeyCode_ArrowUp,    KeyModifier_Shift, Command_SelectArrowUp);
    BindKeyCommand(CommandTable, KeyCode_ArrowDown,  KeyModifier_Shift, Command_SelectArrowDown);
    
    BindKeyCommand(CommandTable, KeyCode_ArrowRight, KeyModifier_Ctrl,  Command_GoToNextToken);
    BindKeyCommand(CommandTable, KeyCode_ArrowLeft,  KeyModifier_Ctrl,  Command_GoToPreviousToken);
    BindKeyCommand(CommandTable, KeyCode_ArrowUp,    KeyModifier_Ctrl,  Command_ScrollPanelUp);
    BindKeyCommand(CommandTable, KeyCode_ArrowDown,  KeyModifier_Ctrl,  Command_ScrollPanelDown);

    BindKeyCommand(CommandTable,
                   KeyCode_ArrowRight, KeyModifier_Ctrl | KeyModifier_Shift,
                   Command_SelectUntilNextToken);
    BindKeyCommand(CommandTable,
                   KeyCode_ArrowLeft, KeyModifier_Ctrl | KeyModifier_Shift,
                   Command_SelectUntilPreviousToken);

    BindKeyCommand(CommandTable, KeyCode_PageUp,     KeyModifier_None,  Command_PageUp);
    BindKeyCommand(CommandTable, KeyCode_PageDown,   KeyModifier_None,  Command_PageDown);

    //
    // NOTE(traian): Text manipulation.
    //

    BindKeyCommand(CommandTable, KeyCode_Backspace,  KeyModifier_None,  Command_RemoveCharacterFromLeft);
    BindKeyCommand(CommandTable, KeyCode_Backspace,  KeyModifier_Shift, Command_RemoveCharacterFromLeft);
    BindKeyCommand(CommandTable, KeyCode_Backspace,  KeyModifier_Ctrl,  Command_RemoveCharactersUntilPreviousToken);
    BindKeyCommand(CommandTable, KeyCode_Delete,     KeyModifier_None,  Command_RemoveCharacterFromRight);
    BindKeyCommand(CommandTable, KeyCode_Delete,     KeyModifier_Ctrl,  Command_RemoveCharactersUntilNextToken);

    //
    // NOTE(traian): File management.
    //

    BindKeyCommand(CommandTable, 'S', KeyModifier_Ctrl, Command_SaveFile);

    //
    // NOTE(traian): Editor management.
    //

    BindKeyCommand(CommandTable, KeyCode_FKeyFirst + 3,  KeyModifier_Alt,  Command_Quit);
    BindKeyCommand(CommandTable, KeyCode_FKeyFirst + 10, KeyModifier_None, Command_ToggleFullscreen);

    BindKeyCommand(CommandTable, KeyCode_One,            KeyModifier_Alt,  Command_FocusOnTextPanelOne);
    BindKeyCommand(CommandTable, KeyCode_Two,            KeyModifier_Alt,  Command_FocusOnTextPanelTwo);
    BindKeyCommand(CommandTable, KeyCode_Three,          KeyModifier_Alt,  Command_FocusOnTextPanelThree);
    BindKeyCommand(CommandTable, KeyCode_Four,           KeyModifier_Alt,  Command_FocusOnTextPanelFour);
}

internal inline editor_command_function *
GetEditorCommandCallback(command_table *CommandTable, key_modifier Modifiers, u32 EntryIndex)
{
    editor_command_function *Callback = NULL;
    while (EntryIndex != UINT32_MAX)
    {
        command_entry *Entry = CommandTable->CommandEntriesPool + EntryIndex;
        if (Entry->Modifiers == Modifiers)
        {
            Callback = Entry->Callback;
            break;
        }

        EntryIndex = Entry->NextIndex;
    }

    return Callback;
}

void
ExecuteEditorCommand(editor_state *EditorState, key_code KeyCode)
{
    key_modifier Modifiers = PlatformGetKeyModifiers();
    b32 IsCapsLockActive = PlatformIsCapsLockActive();

    command_table *CommandTable = &EditorState->CommandTable;
    u32 EntryIndex = CommandTable->KeyCommands[KeyCode];
    editor_command_function *Callback = GetEditorCommandCallback(CommandTable, Modifiers, EntryIndex);
    
    if (Callback)
    {
        editor_command_info CommandInfo = {};
        CommandInfo.KeyCode = KeyCode;
        CommandInfo.Modifiers = Modifiers;
        CommandInfo.IsCapsLockActive = IsCapsLockActive;
        CommandInfo.OpaqueData = NULL;

        Callback(EditorState, EditorState->FocusedTextPanelIndex, &CommandInfo);
    }
}
