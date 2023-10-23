/*  =====================================================================
    $File:   ocean.cpp $
    $Date:   September 17 2023 $
    $Author: Traian Avram $
    $Notice: Copyright (c) 2023-2023 Traian Avram. All Rights Reserved. $
    =====================================================================  */

/*
    KNOWN BUGS:

    FEATURES TO IMPLEMENT:
        - File explorer

    OPTIMIZATIONS TO IMPLEMENT:
        - Only redraw the text lines that were modified. 
        - Perform layout modification/draw updates only when neccessary by
          handling each window event type differently - currently, the editor
          is redrawn from scratch each time a window event is received.
        - Make each font glyph texture the same size and store all of the
          ASCII glyphs into a single linear texture buffer. This should greatly
          improve the cache coherency.
    
    BUGS TO FIX:
*/

#include "ocean.h"
#include "ocean_math.h"

#include "ocean_commands.cpp"

#define STB_TRUETYPE_IMPLEMENTATION 1
#include "stb_truetype.h"

//=========================================================================================
// NOTE(traian): MEMORY.
//=========================================================================================

void
CopyMem(void *Destination, void *Source, memory_size Size)
{
    u8 *Dst = (u8 *)Destination;
    u8 *Src = (u8 *)Source;
    while (Size--)
    {
        *Dst++ = *Src++;
    }
}

void
SetMemory(void *Destination, u8 Value, memory_size Size)
{
    u8 *Dst = (u8 *)Destination;
    while (Size--)
    {
        *Dst++ = Value;
    }
}

void
SetMemoryToZero(void *Destination, memory_size Size)
{
    u8 *Dst = (u8 *)Destination;
    while (Size--)
    {
        *Dst++ = 0;
    }
}

void
InitializeArena(memory_arena *Arena, void *Memory, memory_size Size)
{
    Arena->Base = (u8 *)Memory;
    Arena->Size = Size;
    Arena->Offset = 0;
}

u8 *
AllocateFromArena(memory_arena *Arena, memory_size Size)
{
    if (Size == 0)
    {
        return NULL;
    }

    Assert(Arena->Offset + Size <= Arena->Size);
    u8 *Result = Arena->Base + Arena->Offset;
    Arena->Offset += Size;
    return Result;
}

void
ResetArena(memory_arena *Arena)
{
    SetMemoryToZero(Arena->Base, Arena->Offset);
    Arena->Offset = 0;
}

temporary_arena
BeginTemporaryArena(memory_arena *Arena)
{
    temporary_arena Result;
    
    Result.ParentArena = Arena;
    Result.Arena.Base = Arena->Base + Arena->Offset;
    Result.Arena.Size = Arena->Size - Arena->Offset;
    Result.Arena.Offset = 0;

    Arena->Offset = Arena->Size;
    return Result;
}

void
EndTemporaryArena(temporary_arena *TemporaryArena)
{
    memory_arena *ParentArena = TemporaryArena->ParentArena;
    Assert(ParentArena->Offset == ParentArena->Size);
    memory_arena *Arena = &TemporaryArena->Arena;

    SetMemoryToZero(Arena->Base, Arena->Size);
    ParentArena->Offset = Arena->Base - ParentArena->Base;

    Arena->Base = NULL;
    Arena->Size = 0;
    Arena->Offset = 0;
}


//=========================================================================================
// NOTE(traian): BITMAPS.
//=========================================================================================

internal void
AllocateBitmap(bitmap *Bitmap, memory_arena *Arena, u32 Width, u32 Height, u32 BytesPerPixel)
{
    Bitmap->Width = Width;
    Bitmap->Height = Height;
    Bitmap->BytesPerPixel = BytesPerPixel;
    Bitmap->Pitch = Width * BytesPerPixel;

    memory_size BitmapByteCount = Width * Height * BytesPerPixel;
    Bitmap->Memory = PushSize(Arena, BitmapByteCount);
}

internal u8 *
GetPixelAddress(u8 *Pixels, u32 BytesPerPixel, u32 Pitch, u32 PositionX, u32 PositionY)
{
    u8 *Result = Pixels + ((PositionY * Pitch) + (PositionX * BytesPerPixel));
    return Result;
}

//=========================================================================================
// NOTE(traian): FONTS.
//=========================================================================================

internal void
LoadFont(memory_arena *Arena, font *Font, buffer TTFBuffer, float Height)
{
    Font->Height = Height;

    stbtt_fontinfo FontInfo;
    stbtt_InitFont(&FontInfo, TTFBuffer.Data, stbtt_GetFontOffsetForIndex(TTFBuffer.Data, 0));
    float Scale = stbtt_ScaleForPixelHeight(&FontInfo, Height);
    
    int Advance;
    stbtt_GetCodepointHMetrics(&FontInfo, 'X', &Advance, NULL);
    Font->Advance = (u32)((f32)Advance * Scale);

    int Ascent, Descent, LineGap;
    stbtt_GetFontVMetrics(&FontInfo, &Ascent, &Descent, &LineGap);
    Font->Ascent = (u32)((f32)Ascent * Scale);
    Font->Descent = (u32)((f32)-Descent * Scale);
    Font->LineGap = (u32)((f32)LineGap * Scale);

    for (u32 CodepointIndex = 0;
        CodepointIndex < FONT_ASCII_COUNT;
        ++CodepointIndex)
    {
        char Codepoint = FONT_ASCII_OFFSET + CodepointIndex;
        int Width, Height, OffsetX, OffsetY;
        u8 *FontBitmap = stbtt_GetCodepointBitmap(&FontInfo, 0, Scale,
                                                  Codepoint, &Width, &Height, &OffsetX, &OffsetY);

        font_entry *Entry = Font->ASCIIEntries + CodepointIndex;
        Entry->OffsetX = OffsetX;
        // NOTE(traian): The stbtt generated bitmap is Y-down (top-left origin).
        Entry->OffsetY = -(Height + OffsetY);
        AllocateBitmap(&Entry->Bitmap, Arena, Width, Height, 1);

        u8 *DestRow = GetPixelAddress(Entry->Bitmap.Memory, 1, Entry->Bitmap.Pitch,
                                      0, Entry->Bitmap.Height - 1);
        u8 *Source = FontBitmap;

        for (u32 Y = 0; Y < Height; ++Y)
        {
            for (u32 X = 0; X < Width; ++X)
            {
                DestRow[X] = *Source++;
            }

            DestRow -= Entry->Bitmap.Pitch;
        }
    }
}

internal font *
GetFontFromID(editor_state *EditorState, font_id FontID)
{
    Assert(FontID < FontID_MaxCount);
    u32 FontIndex = EditorState->FontTable.IndexMap[FontID];
    font *Result = EditorState->FontTable.Fonts + FontIndex;
    return Result;
}

//=========================================================================================
// NOTE(traian): PRIMITIVE DRAWING UTILITIES.
//=========================================================================================

internal void
DrawQuad(bitmap *OffscreenBitmap, offset2 Offset, extent2 Extent, u32 PackedColor)
{
    Assert(Offset.X + Extent.Width <= OffscreenBitmap->Width);
    Assert(Offset.Y + Extent.Height <= OffscreenBitmap->Height);

    u32 *Destination = (u32 *)GetPixelAddress(OffscreenBitmap->Memory,
                                              OffscreenBitmap->BytesPerPixel, OffscreenBitmap->Pitch,
                                              Offset.X, Offset.Y);
    for (u32 Y = 0; Y < Extent.Height; ++Y)
    {
        for (u32 X = 0; X < Extent.Width; ++X)
        {
            Destination[X] = PackedColor;
        }

        Destination += OffscreenBitmap->Width;
    }
}

internal u32
BlendColors(u8 SrcR, u8 SrcG, u8 SrcB, u32 DestinationColor, u32 Alpha)
{
    u8 DstR, DstG, DstB;
    UnpackRGBA(DestinationColor, &DstR, &DstG, &DstB);

    f32 AlphaS = (f32)Alpha / 255.0F;
    f32 AlphaD = 1.0F - AlphaS;

    u8 BlendedR = (u8)((f32)SrcR * AlphaS + DstR * AlphaD);
    u8 BlendedG = (u8)((f32)SrcG * AlphaS + DstG * AlphaD);
    u8 BlendedB = (u8)((f32)SrcB * AlphaS + DstB * AlphaD);

    u32 BlendedColor = PackRGBA(BlendedR, BlendedG, BlendedB, 0xFF);
    return BlendedColor;
}

internal u32
BlendColors(u32 SourceColor, u32 DestinationColor, u32 Alpha)
{
    u8 SrcR, SrcG, SrcB;
    UnpackRGBA(SourceColor, &SrcR, &SrcG, &SrcB);

    u32 Blended = BlendColors(SrcR, SrcG, SrcB, DestinationColor, Alpha);
    return Blended;
}

internal void
DrawTransparentQuad(bitmap *OffscreenBitmap, offset2 Offset, extent2 Extent, u32 PackedColor)
{
    Assert(Offset.X + Extent.Width <= OffscreenBitmap->Width);
    Assert(Offset.Y + Extent.Height <= OffscreenBitmap->Height);

    u8 R, G, B, A;
    UnpackRGBA(PackedColor, &R, &G, &B, &A);
    f32 AlphaS = (f32)A / 255.0F;
    f32 AlphaD = 1.0F - AlphaS;

    u32 *Destination = (u32 *)GetPixelAddress(OffscreenBitmap->Memory,
                                              OffscreenBitmap->BytesPerPixel, OffscreenBitmap->Pitch,
                                              Offset.X, Offset.Y);
    for (u32 Y = 0; Y < Extent.Height; ++Y)
    {
        for (u32 X = 0; X < Extent.Width; ++X)
        {
            u32 DestColor = Destination[X];
            u32 Blended = BlendColors(R, G, B, DestColor, A);
            Destination[X] = Blended;
        }

        Destination += OffscreenBitmap->Width;
    }
}

internal void
DrawFontBitmap(bitmap *OffscreenBitmap, bitmap *FontBitmap, offset2 Offset, u8 R, u8 G, u8 B)
{
    Assert(Offset.X + FontBitmap->Width <= OffscreenBitmap->Width);
    Assert(Offset.Y + FontBitmap->Height <= OffscreenBitmap->Height);

    u32 *Dst = (u32 *)GetPixelAddress(OffscreenBitmap->Memory,
                                              OffscreenBitmap->BytesPerPixel, OffscreenBitmap->Pitch,
                                              Offset.X, Offset.Y);
    u8 *Src = FontBitmap->Memory;
    
    for (u32 Y = 0; Y < FontBitmap->Height; ++Y)
    {
        for (u32 X = 0; X < FontBitmap->Width; ++X)
        {
            u32 Blended = BlendColors(R, G, B, Dst[X], *Src++);
            Dst[X] = Blended;
        }

        Dst += OffscreenBitmap->Width;
    }
}

internal void
DrawTextLine(bitmap *OffscreenBitmap, font *Font, char *Text, u32 TextCount, offset2 Offset, u32 PackedColor)
{
    s32 X = Offset.X;
    s32 Y = Offset.Y + Font->Descent;

    u8 R, G, B;
    UnpackRGBA(PackedColor, &R, &G, &B);

    for (u32 Index = 0; Index < TextCount; ++Index)
    {
        char C = Text[Index];
        if (FONT_ASCII_OFFSET <= C && C <= FONT_ASCII_OFFSET + FONT_ASCII_COUNT)
        {
            font_entry *Entry = Font->ASCIIEntries + (C - FONT_ASCII_OFFSET);
            DrawFontBitmap(OffscreenBitmap, &Entry->Bitmap,
                           { X + Entry->OffsetX, Y + Entry->OffsetY },
                           R, G, B);

            X += Font->Advance;
        }
        else if (C == ' ')
        {
            X += Font->Advance;
        }
    }
}

//=========================================================================================
// NOTE(traian): EDITOR LAYOUT CALCULATIONS.
//=========================================================================================

internal rectangle2
GetStatusBarSurface(editor_state *EditorState, u32 PanelIndex)
{
    text_panel *Panel = EditorState->TextPanels + PanelIndex;

    rectangle2 Result;
    Result.Extent.Width = Panel->Surface.Extent.Width;
    Result.Extent.Height = EditorState->Settings.StatusBarHeight;
    Result.Offset.X = Panel->Surface.Offset.X;
    Result.Offset.Y = Panel->Surface.Offset.Y - EditorState->Settings.StatusBarHeight;

    return Result;
}

struct screen_matrix_result
{
    u32 LineCount;
    u32 NextLineSpace;
    u32 ColumnCount;
};

internal screen_matrix_result
CalculateScreenMatrix(rectangle2 PanelSurface, editor_state *EditorState)
{
    screen_matrix_result Result = {};
    u32 AvailableWidth = PanelSurface.Extent.Width - EditorState->Settings.TextPaddingX;
    u32 AvailableHeight = PanelSurface.Extent.Height - EditorState->Settings.TextPaddingY;

    font *Font = GetFontFromID(EditorState, FontID_Text);
    u32 TextHeight = Font->Ascent + Font->Descent;
    u32 LineGap = Font->LineGap;

    Result.LineCount = (AvailableHeight + LineGap) / (TextHeight + LineGap);
    u32 RemainingSpace = AvailableHeight - (Result.LineCount * (TextHeight + LineGap));
    if (RemainingSpace > LineGap)
    {
        Result.NextLineSpace = RemainingSpace - LineGap;
    }

    Result.ColumnCount = AvailableWidth / Font->Advance;

    return Result;
}

internal void
ResetEditorLayout(editor_state *EditorState, editor_layout NewLayout)
{
    editor_settings *Settings = &EditorState->Settings;

    switch (NewLayout)
    {
        case EditorLayout_Dual:
        {
            text_panel *LPanel = EditorState->TextPanels + 0;
            text_panel *RPanel = EditorState->TextPanels + 1;

            u32 AvailableWidth = EditorState->WindowWidth - Settings->SeparatorThickness;
            u32 AvailableHeight = EditorState->WindowHeight - Settings->StatusBarHeight;

            LPanel->Surface.Offset.X = 0;
            LPanel->Surface.Offset.Y = Settings->StatusBarHeight;
            LPanel->Surface.Extent.Width = AvailableWidth / 2;
            LPanel->Surface.Extent.Height = AvailableHeight;
            screen_matrix_result LScreenMatrix = CalculateScreenMatrix(LPanel->Surface, EditorState);
            LPanel->ScreenLineCount = LScreenMatrix.LineCount;
            LPanel->ScreenColumnCount = LScreenMatrix.ColumnCount;

            RPanel->Surface.Offset.X = LPanel->Surface.Extent.Width + Settings->SeparatorThickness;
            RPanel->Surface.Offset.Y = LPanel->Surface.Offset.Y;
            RPanel->Surface.Extent.Width = (AvailableWidth / 2) + (AvailableWidth % 2);
            RPanel->Surface.Extent.Height = LPanel->Surface.Extent.Height = AvailableHeight;
            screen_matrix_result RScreenMatrix = CalculateScreenMatrix(RPanel->Surface, EditorState);
            RPanel->ScreenLineCount = RScreenMatrix.LineCount;
            RPanel->ScreenColumnCount = RScreenMatrix.ColumnCount;

        } break;
    }

    EditorState->EditorLayout = NewLayout;
}

//=========================================================================================
// NOTE(traian): EDITOR WIDGET DRAWING.
//=========================================================================================

internal void
WidgetPainter_ClearStatusBar(bitmap *OffscreenBitmap, editor_settings *Settings,
                             rectangle2 StatusBarSurface, b32 ClearBorder = false)
{
    offset2 Offset = StatusBarSurface.Offset;
    extent2 Extent = StatusBarSurface.Extent;
    DrawQuad(OffscreenBitmap, Offset, Extent, Settings->StatusBarColor);
}

internal void
WidgetPainter_ClearPanel(bitmap *OffscreenBitmap, editor_state *EditorState, u32 PanelIndex)
{
    text_panel *Panel = EditorState->TextPanels + PanelIndex;
    DrawQuad(OffscreenBitmap,
             Panel->Surface.Offset, Panel->Surface.Extent, EditorState->Settings.BackgroundColor);
}

internal u8
GetDigitsCount(u32 Number)
{
    u8 Result = 1;
    if (Number > 0)
    {
        Result = 0;
        while (Number > 0)
        {
            ++Result;
            Number /= 10;
        }
    }

    return Result;
}

internal void
WidgetPainter_StatusBarText(bitmap *OffscreenBitmap, editor_state *EditorState, u32 PanelIndex,
                            rectangle2 StatusBarSurface)
{
    editor_settings *Settings = &EditorState->Settings;
    text_panel *Panel = EditorState->TextPanels + PanelIndex;
    WidgetPainter_ClearStatusBar(OffscreenBitmap, Settings, StatusBarSurface);

    char *FileName = Panel->FileName ? Panel->FileName : "*unsaved*";

    font *Font = GetFontFromID(EditorState, FontID_Interface);
    u32 TextHeight = Font->Ascent + Font->Descent;
    u32 TextPadding = (StatusBarSurface.Extent.Height - TextHeight) / 2;

    offset2 Offset;
    Offset.X = StatusBarSurface.Offset.X + 8;
    Offset.Y = StatusBarSurface.Offset.Y + TextPadding;

    char Whitespace[16] = {};
    u32 WhitespaceCount = GetDigitsCount(Panel->LineCount + 1) - GetDigitsCount(Panel->Caret.Position.Line + 1);
    SetMemory(Whitespace, ' ', WhitespaceCount);

    char DirtyMark[2] = { '\0', '\0' };
    if (Panel->IsSaveDirty)
    {
        DirtyMark[0] = '*';
    }

    char TitleBuffer[256] = {};
    int Count = sprintf_s(TitleBuffer, sizeof(TitleBuffer), "%s%s - L#%d%s C#%d",
                          FileName, DirtyMark,
                          Panel->Caret.Position.Line + 1, Whitespace, Panel->Caret.Position.Column + 1);
    Assert(Count < sizeof(TitleBuffer));

    DrawTextLine(OffscreenBitmap, Font, TitleBuffer, Count, Offset, Settings->StatusBarTextColor);
}

internal void
WidgetPainter_Separator(bitmap *OffscreenBitmap, editor_state *EditorState)
{
    editor_settings *Settings = &EditorState->Settings;

    switch (EditorState->EditorLayout)
    {
        case EditorLayout_Dual:
        {
            text_panel *LPanel = EditorState->TextPanels + 0;
            text_panel *RPanel = EditorState->TextPanels + 1;
            Assert(LPanel->Surface.Extent.Width +
                   RPanel->Surface.Extent.Width +
                   Settings->SeparatorThickness == EditorState->WindowWidth)

            DrawQuad(OffscreenBitmap,
                     { LPanel->Surface.Extent.Width, 0 },
                     { (s32)Settings->SeparatorThickness, (s32)EditorState->WindowHeight },
                     Settings->SeparatorColor);
        } break;
    }
}

internal offset2
GetCharacterDrawOffset(editor_state *EditorState, rectangle2 PanelSurface, u32 Line, u32 Column)
{
    font *Font = GetFontFromID(EditorState, FontID_Text);
    u32 FontHeight = Font->Ascent + Font->Descent;

    offset2 Result;
    Result.X = PanelSurface.Offset.X + EditorState->Settings.TextPaddingX + (Column * Font->Advance);
    Result.Y = PanelSurface.Offset.Y + PanelSurface.Extent.Height -
                   FontHeight - EditorState->Settings.TextPaddingY -
                   (Line * (FontHeight + Font->LineGap));

    return Result;
}

internal void
WidgetPainter_PanelContent(bitmap *OffscreenBitmap, editor_state *EditorState, u32 PanelIndex)
{
    editor_settings *Settings = &EditorState->Settings;
    text_panel *Panel = EditorState->TextPanels + PanelIndex;

    font *Font = GetFontFromID(EditorState, FontID_Text);
    u32 TextHeight = Font->Ascent + Font->Descent;

    u32 MaxAvailableWidth = Panel->Surface.Offset.X + Panel->Surface.Extent.Width;
    offset2 Position = GetCharacterDrawOffset(EditorState, Panel->Surface, 0, 0);
    Position.Y += Font->Descent;
    u32 XResetPoint = Position.X;

    u8 R, G, B;
    UnpackRGBA(Settings->TextColor, &R, &G, &B);

    text_iterator Iterator = NewTextIterator(&Panel->Buffer, Panel->BufferOffset);
    for (u32 LineIndex = 0; LineIndex < Panel->ScreenLineCount; ++LineIndex)
    {
        b32 SkipLine = false;
        u32 FirstColumnIndex = 0;
        while (FirstColumnIndex < Panel->FirstColumnIndex)
        {
            if (!IsValid(Iterator))
            {
                SkipLine = true;
                break;
            }
            if (Iterator.Codepoint == '\n')
            {
                SkipLine = true;
                Iterator = AdvanceIterator(Iterator);
                break;
            }

            FirstColumnIndex += GetCodepointColumnCount(Settings, Iterator.Codepoint, FirstColumnIndex);
            Iterator = AdvanceIterator(Iterator);
        }

        b32 ReachedEndOfLine = false;
        if (!SkipLine)
        {
            u32 EmptyColumns = FirstColumnIndex - Panel->FirstColumnIndex;
            Position.X += EmptyColumns * Font->Advance;
            for (u32 ColumnOffset = 0; ColumnOffset < Panel->ScreenColumnCount - EmptyColumns;)
            {
                if (!IsValid(Iterator))
                {
                    ReachedEndOfLine = true;
                    break;
                }
                if (Iterator.Codepoint == '\n')
                {
                    ReachedEndOfLine = true;
                    Iterator = AdvanceIterator(Iterator);
                    break;
                }

                if (IsDrawableCodepoint(Iterator.Codepoint))
                {
                    font_entry *Entry = Font->ASCIIEntries + (Iterator.Codepoint - FONT_ASCII_OFFSET);
                    DrawFontBitmap(OffscreenBitmap, &Entry->Bitmap,
                                   { Position.X + Entry->OffsetX, Position.Y + Entry->OffsetY },
                                   R, G, B);
                }

                u32 CodepointColumnCount = GetCodepointColumnCount(Settings, Iterator.Codepoint,
                                                                   FirstColumnIndex + ColumnOffset);
                Position.X += CodepointColumnCount * Font->Advance;
                ColumnOffset += CodepointColumnCount;

                Iterator = AdvanceIterator(Iterator);
            }

            if (!ReachedEndOfLine)
            {
                Iterator = GetNextLineFirstIterator(Iterator);
            }
        }

        Position.X = XResetPoint;
        Position.Y -= (TextHeight + Font->LineGap);
    }
}

internal void
WidgetPainter_LineHighlight(bitmap *OffscreenBitmap, editor_state *EditorState, u32 PanelIndex)
{
    text_panel *Panel = EditorState->TextPanels + PanelIndex;
    u32 LineIndex = Panel->Caret.Position.Line;

    if (Panel->FirstLineIndex <= LineIndex && LineIndex <= Panel->FirstLineIndex + Panel->ScreenLineCount)
    {
        font *Font = GetFontFromID(EditorState, FontID_Text);
        u32 FontHeight = Font->Ascent + Font->Descent;
        u32 RelativeLine = LineIndex - Panel->FirstLineIndex;

        rectangle2 Highlight;
        Highlight.Offset.X = Panel->Surface.Offset.X;
        Highlight.Offset.Y = GetCharacterDrawOffset(EditorState, Panel->Surface, RelativeLine, 0).Y;
        Highlight.Extent.Width = Panel->Surface.Extent.Width;
        Highlight.Extent.Height = FontHeight + Font->LineGap;

        DrawTransparentQuad(OffscreenBitmap,
                            Highlight.Offset, Highlight.Extent,
                            EditorState->Settings.LineHighlightColor);
    }
}

internal void
WidgetPainter_Caret(bitmap *OffscreenBitmap, editor_state *EditorState, u32 PanelIndex)
{
    text_panel *Panel = EditorState->TextPanels + PanelIndex;
    u32 LineIndex = Panel->Caret.Position.Line;
    u32 ColumnIndex = Panel->Caret.Position.Column;

    if (Panel->FirstLineIndex <= LineIndex && LineIndex <= Panel->FirstLineIndex + Panel->ScreenLineCount &&
        Panel->FirstColumnIndex <= ColumnIndex && ColumnIndex <= Panel->FirstColumnIndex + Panel->ScreenColumnCount)
    {
        font *Font = GetFontFromID(EditorState, FontID_Text);
        u32 FontHeight = Font->Ascent + Font->Descent;
        u32 RelativeLine = Panel->Caret.Position.Line - Panel->FirstLineIndex;
        u32 RelativeColumn = Panel->Caret.Position.Column - Panel->FirstColumnIndex;

        rectangle2 Position;
        Position.Offset = GetCharacterDrawOffset(EditorState, Panel->Surface, RelativeLine, RelativeColumn);
        Position.Extent.Width = EditorState->Settings.CaretWidth;
        Position.Extent.Height = FontHeight + Font->LineGap;

        DrawTransparentQuad(OffscreenBitmap,
                            Position.Offset, Position.Extent,
                            EditorState->Settings.CaretColor);
    }
}

internal void
WidgetPainter_SelectionHighlight(bitmap *OffscreenBitmap, editor_state *EditorState, u32 PanelIndex)
{
    text_panel *Panel = EditorState->TextPanels + PanelIndex;
    font *Font = GetFontFromID(EditorState, FontID_Text);
    u32 FontHeight = Font->Ascent + Font->Descent;

    if (Panel->Caret.IsSelecting)
    {
        u32 Line, Column;
        memory_offset BufferOffset, TargetOffset;

        if (Panel->Caret.Position.Offset > Panel->Caret.Selection.Offset)
        {
            Line = Panel->Caret.Selection.Line;
            Column = Panel->Caret.Selection.Column;
            BufferOffset = Panel->Caret.Selection.Offset;
            TargetOffset = Panel->Caret.Position.Offset;
        }
        else
        {
            Line = Panel->Caret.Position.Line;
            Column = Panel->Caret.Position.Column;
            BufferOffset = Panel->Caret.Position.Offset;
            TargetOffset = Panel->Caret.Selection.Offset;
        }

        text_iterator Iterator = NewTextIterator(&Panel->Buffer, BufferOffset);
        while (BufferOffset < TargetOffset)
        {
            if ((Panel->FirstLineIndex <= Line && Line < Panel->FirstLineIndex + Panel->ScreenLineCount) &&
                (Panel->FirstColumnIndex <= Column && Column < Panel->FirstColumnIndex + Panel->ScreenColumnCount))
            {
                u32 RelativeLine = Line - Panel->FirstLineIndex;
                u32 RelativeColumn = Column - Panel->FirstColumnIndex;
                u32 ColumnCount = GetCodepointColumnCount(&EditorState->Settings, Iterator.Codepoint, Column);

                DrawTransparentQuad(OffscreenBitmap,
                                    GetCharacterDrawOffset(EditorState, Panel->Surface, RelativeLine, RelativeColumn),
                                    { (s32)ColumnCount * (s32)Font->Advance, (s32)FontHeight },
                                    PackRGBA(230, 150, 170, 100));
            }

            if (Iterator.Codepoint == '\n')
            {
                Column = 0;
                ++Line;
            }
            else
            {
                Column += GetCodepointColumnCount(&EditorState->Settings, Iterator.Codepoint, Column);
            }

            BufferOffset += Iterator.Width;
            Iterator = AdvanceIterator(Iterator);
        }
    }
}

//=========================================================================================
// NOTE(traian): EDITOR INITIALIZATION.
//=========================================================================================

internal void
InitializeColorScheme(editor_settings *Settings)
{
    Settings->TextColor           = PackRGBA(78,  201, 176, 255);
    Settings->BackgroundColor     = PackRGBA(23,  23,  23,  255);
    Settings->LineHighlightColor  = PackRGBA(40,  60,  210, 60);
    Settings->CaretColor          = PackRGBA(220, 220, 60,  225);

    // Settings->StatusBarColor      = PackRGBA(135);
    // Settings->StatusBarTextColor  = PackRGBA(8);
    // Settings->SeparatorColor      = PackRGBA(175);
    
    Settings->StatusBarColor      = PackRGBA(195, 0,  82);
    Settings->StatusBarTextColor  = PackRGBA(12);
    Settings->SeparatorColor      = PackRGBA(235, 40, 122);

    // Settings->StatusBarColor      = PackRGBA(0, 99,  177);
    // Settings->StatusBarTextColor  = PackRGBA(235);
    // Settings->SeparatorColor      = PackRGBA(40, 139,  177);
}

internal void
InitializeFonts(editor_state *EditorState, editor_memory *EditorMemory)
{
    memory_arena *Arena = &EditorMemory->PermanentArena;
    font_table *FontTable = &EditorState->FontTable;

    // TODO(traian): Embed this file into the executable.
    buffer TTFBuffer = PlatformReadEntireFile("../content/CascadiaMono-SemiLight.ttf", Arena);
    Assert(TTFBuffer.Data);

    FontTable->FontCount = 2;
    FontTable->Fonts = PushArray(Arena, font, FontTable->FontCount);
    
    // NOTE(traian): Rasterize the font glyphs.
    LoadFont(Arena, FontTable->Fonts + 0, TTFBuffer, 22.0F);
    LoadFont(Arena, FontTable->Fonts + 1, TTFBuffer, 20.0F);

    // NOTE(traian): Initialize the font index map.
    FontTable->IndexMap[FontID_Default] = 0;
    FontTable->IndexMap[FontID_Text] = 0;
    FontTable->IndexMap[FontID_Interface] = 1;
}

internal void
OpenDefaultFiles(editor_state *EditorState, editor_memory *EditorMemory)
{
    text_panel *LPanel = EditorState->TextPanels + 0;
    text_panel *RPanel = EditorState->TextPanels + 1;
    command_open_file_data CommandData;
    editor_command_info CommandInfo = {};
    CommandInfo.OpaqueData = &CommandData;

    // CommandData.FileName = "C:/Dev/ShooterEngine/Source/Core/CoreTypes.h";
    CommandData.FileName = "C:/Dev/OceanVulkan/source/ocean_vulkan.cpp";
    Command_OpenFile(EditorState, 0, &CommandInfo);

    // CommandData.FileName = "C:/Dev/ShooterEngine/Source/Core/Array.h";
    CommandData.FileName = "C:/Dev/OceanVulkan/source/ocean_vulkan.h";
    Command_OpenFile(EditorState, 1, &CommandInfo);
    // Command_NewTextBuffer(EditorState, 1, NULL);
}

void
InitializeEditor(editor_state *EditorState, editor_memory *EditorMemory)
{
    InitializeFonts(EditorState, EditorMemory);
    InitializeEditorCommandTable(EditorState);

    editor_settings *Settings = &EditorState->Settings;
    InitializeColorScheme(Settings);

    Settings->CaretWidth = 2;
    Settings->StatusBarHeight = 25;
    Settings->SeparatorThickness = 2;
    Settings->TextPaddingX = 4;
    Settings->TextPaddingY = 2;
    Settings->TabWidth = 4;
    Settings->ReplaceTabWithSpaces = true;

    OpenDefaultFiles(EditorState, EditorMemory);
    EditorState->FocusedTextPanelIndex = 0;
}

//=========================================================================================
// NOTE(traian): EDITOR UPDATE AND RENDER.
//=========================================================================================

void
UpdateAndRenderEditor(editor_state *EditorState, editor_memory *EditorMemory,
                      bitmap *OffscreenBitmap)
{
    ResetEditorLayout(EditorState, EditorLayout_Dual);

    rectangle2 StatusBarSurface0 = GetStatusBarSurface(EditorState, 0);
    WidgetPainter_ClearStatusBar(OffscreenBitmap, &EditorState->Settings, StatusBarSurface0, true);
    WidgetPainter_StatusBarText(OffscreenBitmap, EditorState, 0, StatusBarSurface0);
    WidgetPainter_ClearPanel(OffscreenBitmap, EditorState, 0);
    WidgetPainter_PanelContent(OffscreenBitmap, EditorState, 0);
    WidgetPainter_LineHighlight(OffscreenBitmap, EditorState, 0);
    WidgetPainter_Caret(OffscreenBitmap, EditorState, 0);
    WidgetPainter_SelectionHighlight(OffscreenBitmap, EditorState, 0);

    rectangle2 StatusBarSurface1 = GetStatusBarSurface(EditorState, 1);
    WidgetPainter_ClearStatusBar(OffscreenBitmap, &EditorState->Settings, StatusBarSurface1, true);
    WidgetPainter_StatusBarText(OffscreenBitmap, EditorState, 1, StatusBarSurface1);
    WidgetPainter_ClearPanel(OffscreenBitmap, EditorState, 1);
    WidgetPainter_PanelContent(OffscreenBitmap, EditorState, 1);
    WidgetPainter_LineHighlight(OffscreenBitmap, EditorState, 1);
    WidgetPainter_Caret(OffscreenBitmap, EditorState, 1);
    WidgetPainter_SelectionHighlight(OffscreenBitmap, EditorState, 1);

    WidgetPainter_Separator(OffscreenBitmap, EditorState);
    
    VALIDATE_CARET_OFFSET(EditorState, 0);
    VALIDATE_CARET_OFFSET(EditorState, 1);
}

void
EditorEventKeyPressed(editor_state *EditorState, key_code KeyCode)
{
    ExecuteEditorCommand(EditorState, KeyCode);
}
