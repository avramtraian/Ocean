/*  =====================================================================
    $File:   ocean.cpp $
    $Date:   September 17 2023 $
    $Author: Traian Avram $
    $Notice: Copyright (c) 2023-2023 Traian Avram. All Rights Reserved. $
    =====================================================================  */

#include "ocean.h"
#include "ocean_math.h"

#define STB_TRUETYPE_IMPLEMENTATION 1
#include "stb_truetype.h"

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

internal u8 *
GetPixelAddress(u8 *Pixels, u32 BytesPerPixel, u32 Pitch, u32 PositionX, u32 PositionY)
{
    u8 *Result = Pixels + ((PositionY * Pitch) + (PositionX * BytesPerPixel));
    return Result;
}

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

internal void
ResetEditorLayout(editor_state *EditorState, editor_memory *EditorMemory, editor_layout NewLayout)
{
    switch (NewLayout)
    {
        case EditorLayout_Single:
        {
            text_panel *Panel = EditorState->TextPanels + 0;

            Panel->Width = EditorState->WindowWidth;
            Panel->Height = EditorState->WindowHeight - EditorState->Settings.StatusBarHeight;
            Panel->PositionX = 0;
            Panel->PositionY = 0;
        } break;

        case EditorLayout_Dual:
        {
            text_panel *LeftPanel = EditorState->TextPanels + 0;
            text_panel *RightPanel = EditorState->TextPanels + 1;
            u32 AvailableWidth = EditorState->WindowWidth - EditorState->Settings.SeparatorThickness;
            u32 AvailableHeight = EditorState->WindowHeight - EditorState->Settings.StatusBarHeight;

            LeftPanel->Width = AvailableWidth / 2;
            LeftPanel->Height = AvailableHeight;
            LeftPanel->PositionX = 0;
            LeftPanel->PositionY =  EditorState->Settings.StatusBarHeight;

            RightPanel->Width = (AvailableWidth / 2) + (AvailableWidth % 2);
            RightPanel->Height = AvailableHeight;
            RightPanel->PositionX = LeftPanel->Width + EditorState->Settings.SeparatorThickness;
            RightPanel->PositionY =  EditorState->Settings.StatusBarHeight;
        } break;

        case EditorLayout_Quad:
        {
            text_panel *LBPanel = EditorState->TextPanels + 0;
            text_panel *LTPanel = EditorState->TextPanels + 1;
            text_panel *RBPanel = EditorState->TextPanels + 2;
            text_panel *RTPanel = EditorState->TextPanels + 3;

            u32 AvailableWidth = EditorState->WindowWidth - EditorState->Settings.SeparatorThickness;
            u32 AvailableHeight = EditorState->WindowHeight
                                  - 2 * EditorState->Settings.StatusBarHeight
                                  - EditorState->Settings.SeparatorThickness;

            LBPanel->Width = AvailableWidth / 2;
            LBPanel->Height = AvailableHeight / 2;
            LBPanel->PositionX = 0;
            LBPanel->PositionY = 0;

            LTPanel->Width = LBPanel->Width;
            LTPanel->Height = (AvailableHeight / 2) + (AvailableHeight % 2);
            LTPanel->PositionX = 0;
            LTPanel->PositionY = LBPanel->Height
                                 + EditorState->Settings.StatusBarHeight
                                 + EditorState->Settings.SeparatorThickness;

            RBPanel->Width = (AvailableWidth / 2) + (AvailableWidth % 2);
            RBPanel->Height = LBPanel->Height;
            RBPanel->PositionX = LBPanel->Width + EditorState->Settings.SeparatorThickness;
            RBPanel->PositionY = LBPanel->PositionY;

            RTPanel->Width = RBPanel->Width;
            RTPanel->Height = LTPanel->Height;
            RTPanel->PositionX = RBPanel->PositionX;
            RTPanel->PositionY = LTPanel->PositionY;
        } break;
    }

    EditorState->EditorLayout = NewLayout;
}

struct file_processing_result
{
    u32 LineCount;
};

internal file_processing_result
ProcessFile(buffer FileContents)
{
    file_processing_result Result = {};
    u32 ColumnIndex = 0;

    for (memory_offset Offset = 0; Offset < FileContents.Size; ++Offset)
    {
        char C = FileContents.Data[Offset];
        ++ColumnIndex;
        
        switch (C)
        {
            case '\n':
            {
                ++Result.LineCount;
                ColumnIndex = 0;
            } break;
        }
    }

    return Result;
}

internal void
OpenSourceFile(editor_state *EditorState, memory_arena *Arena, u32 PanelIndex, char *FileName)
{
    text_panel *Panel = EditorState->TextPanels + PanelIndex;
    editor_settings *Setting = &EditorState->Settings;

    buffer FileContents = PlatformReadEntireFile(FileName, Arena);
    Assert(FileContents.Data);

    Panel->FileBuffer.Data = (char *)FileContents.Data;
    Panel->FileBuffer.Size = FileContents.Size;
    Panel->IsDirty = false;
    Panel->FilePath = FileName;

    file_processing_result ProcessingResult = ProcessFile(FileContents);
    Panel->FirstLine = 0;
    Panel->FirstColumn = 0;
    Panel->LineCount = ProcessingResult.LineCount;

    Panel->CaretCount = 1;
    Panel->Carets = PushArray(Arena, caret, Panel->CaretCount);
}

void
InitializeEditor(editor_state *EditorState, editor_memory *EditorMemory)
{
    memory_arena *Arena = &EditorMemory->PermanentArena;

    //
    // NOTE(traian): Fill the editor settings.
    //

    EditorState->Settings.TextColor = PackRGBA(255, 255, 255);
    EditorState->Settings.BackgroundColor = PackRGBA(12, 12, 12);
    EditorState->Settings.TitleColor = PackRGBA(255, 255, 255);

    EditorState->Settings.SeparatorColor = PackRGBA(60, 234, 15);

    EditorState->Settings.StatusBarFocusedColor = PackRGBA(50, 50, 223);
    EditorState->Settings.StatusBarColor = PackRGBA(25, 25, 223);
    EditorState->Settings.StatusBarAccentLightColor = PackRGBA(100, 100, 130);
    EditorState->Settings.StatusBarAccentDarkColor = PackRGBA(15, 15, 60);

    EditorState->Settings.SeparatorThickness = 1;
    EditorState->Settings.StatusBarHeight = 32;
    EditorState->Settings.StatusBarAccentThickness = 4;
    EditorState->Settings.TextFontHeight = 21.0F;

    EditorState->FontTable.FontCount = 1;
    EditorState->FontTable.Fonts = PushArray(Arena, font, EditorState->FontTable.FontCount);

    //
    // NOTE(traian): Load the required fonts.
    //

    buffer TTFBuffer = PlatformReadEntireFile("LiberationMono-Regular.ttf", &EditorMemory->TemporaryArena);
    Assert(TTFBuffer.Data);
    LoadFont(Arena, &EditorState->FontTable.Fonts[0], TTFBuffer, EditorState->Settings.TextFontHeight);

    EditorState->FontTable.IndexMap[FontID_Default] = 0;
    EditorState->FontTable.IndexMap[FontID_Text] = 0;
    EditorState->FontTable.IndexMap[FontID_Interface] = 0;

    //
    // NOTE(traian): Initialize the initial editor layout.
    //

    ResetEditorLayout(EditorState, EditorMemory, EditorLayout_Dual);

    //
    // NOTE(traian): Open files.
    //

    OpenSourceFile(EditorState, Arena, 0, "W:/Ocean/source/win32_ocean.cpp");
    OpenSourceFile(EditorState, Arena, 1, "W:/Ocean/source/ocean_math.h");
}

void
ShutdownEditor(editor_state *EditorState, editor_memory *EditorMemory)
{
}

internal void
DrawFontBitmap(bitmap *OffscreenBitmap, bitmap *Bitmap, u32 PositionX, u32 PositionY)
{
    Assert(Bitmap->BytesPerPixel == 1);
    Assert(PositionX + Bitmap->Width <= OffscreenBitmap->Width);
    Assert(PositionY + Bitmap->Height <= OffscreenBitmap->Height);

    u32 *Dst = (u32 *)GetPixelAddress(OffscreenBitmap->Memory,
                                      OffscreenBitmap->BytesPerPixel, OffscreenBitmap->Pitch,
                                      PositionX, PositionY);
    u8 *Src = (u8 *)Bitmap->Memory;

    for (u32 Y = 0; Y < Bitmap->Height; ++Y)
    {
        for (u32 X = 0; X < Bitmap->Width; ++X)
        {
            f32 AlphaS = (f32)Src[X] / 255.0F;
            f32 AlphaD = 1.0F - AlphaS;
            u8 SrcR = 255;
            u8 SrcG = 255;
            u8 SrcB = 255;

            u8 DstR, DstG, DstB;
            UnpackRGBA(Dst[X], &DstR, &DstG, &DstB);

            u8 BlendedR, BlendedG, BlendedB;
            BlendedR = (u8)(AlphaS * SrcR + AlphaD * DstR);
            BlendedG = (u8)(AlphaS * SrcG + AlphaD * DstG);
            BlendedB = (u8)(AlphaS * SrcB + AlphaD * DstB);

            Dst[X] = PackRGBA(BlendedR, BlendedG, BlendedB);
        }

        Src += Bitmap->Width;
        Dst += OffscreenBitmap->Width;
    }
}

internal void
DrawQuad(bitmap *OffscreenBitmap, u32 PositionX, u32 PositionY, u32 Width, u32 Height, u32 PackedColor)
{
    Assert(PositionX + Width <= OffscreenBitmap->Width);
    Assert(PositionY + Height <= OffscreenBitmap->Height);

    u32 *Destination = (u32 *)GetPixelAddress(OffscreenBitmap->Memory,
                                              OffscreenBitmap->BytesPerPixel, OffscreenBitmap->Pitch,
                                              PositionX, PositionY);
    
    for (u32 Y = 0; Y < Height; ++Y)
    {
        for (u32 X = 0; X < Width; ++X)
        {
            Destination[X] = PackedColor;
        }

        Destination += OffscreenBitmap->Width;
    }
}

internal font *
GetFontFromID(font_table *FontTable, font_id FontID)
{
    Assert(FontID < FontID_MaxCount);
    u32 FontIndex = FontTable->IndexMap[FontID];
    font *Font = FontTable->Fonts + FontIndex;
    return Font;
}

internal void
DrawTextLine(bitmap *OffscreenBitmap, editor_state *EditorState,
               char *String, u32 Count, font_id FontID,
               u32 PositionX, u32 PositionY, u32 PackedColor,
               u32 MaxWidth = UINT32_MAX)
{
    font *Font = GetFontFromID(&EditorState->FontTable, FontID);

    u32 X = PositionX;
    u32 Y = PositionY + Font->Descent;

    for (u32 Index = 0; Index < Count; ++Index)
    {
        if (X + Font->Advance + PositionX > MaxWidth)
        {
            // NOTE(traian): We reached the maximum allowed string width.
            break;
        }

        char C = String[Index];
        if (FONT_ASCII_OFFSET <= C && C <= FONT_ASCII_OFFSET + FONT_ASCII_COUNT)
        {
            font_entry *Entry = Font->ASCIIEntries + (C - FONT_ASCII_OFFSET);
            DrawFontBitmap(OffscreenBitmap, &Entry->Bitmap,
                             X + Entry->OffsetX, Y + Entry->OffsetY);
            X += Font->Advance;
        }
        else if (C == ' ')
        {
            X += Font->Advance;
        }
        else
        {
            // TODO(traian): Handle non-ASCII codepoints and implement
            // support for non-graphic codepoints, such as tabs.
        }
    }
}

internal void
DrawTextPanelStatusBarBorder(bitmap *OffscreenBitmap, editor_state *EditorState, u32 PanelIndex)
{
    text_panel *Panel = EditorState->TextPanels + PanelIndex;
    editor_settings *Settings = &EditorState->Settings;

    u32 PositionX = Panel->PositionX;
    u32 PositionY = Panel->PositionY - Settings->StatusBarHeight;
    u32 Width = Panel->Width;
    u32 Height = Settings->StatusBarHeight;
    u32 Thickness = Settings->StatusBarAccentThickness;

    // NOTE(traian): Bottom.
    DrawQuad(OffscreenBitmap,
             PositionX, PositionY, Width, Thickness,
             Settings->StatusBarAccentDarkColor);

    // NOTE(traian): Top.
    DrawQuad(OffscreenBitmap,
             PositionX,
             PositionY + Height - Thickness,
             Width, Thickness,
             Settings->StatusBarAccentLightColor);
        
    // NOTE(traian): Right.
    DrawQuad(OffscreenBitmap,
             PositionX + Width - Thickness, PositionY + Thickness,
             Thickness, Height - 2 * Thickness,
             Settings->StatusBarAccentDarkColor);

    // NOTE(traian): Left.
    DrawQuad(OffscreenBitmap,
             PositionX, PositionY + Thickness,
             Thickness, Height - 2 * Thickness,
             Settings->StatusBarAccentLightColor);
}

internal void
ClearTextPanelStatusBar(bitmap *OffscreenBitmap, editor_state *EditorState, u32 PanelIndex)
{
    text_panel *Panel = EditorState->TextPanels + PanelIndex;
    editor_settings *Settings = &EditorState->Settings;
    b32 IsFocused = (EditorState->FocusedTextPanel == PanelIndex);

    u32 PositionX = Panel->PositionX;
    u32 PositionY = Panel->PositionY - Settings->StatusBarHeight;
    u32 Width = Panel->Width;
    u32 Height = Settings->StatusBarHeight;

    DrawQuad(OffscreenBitmap,
             PositionX, PositionY, Width, Height,
             IsFocused ? EditorState->Settings.StatusBarFocusedColor : EditorState->Settings.StatusBarColor);

    if (IsFocused)
    {
        DrawTextPanelStatusBarBorder(OffscreenBitmap, EditorState, PanelIndex);
    }
}

internal void
ClearTextPanelBuffer(bitmap *OffscreenBitmap, editor_state *EditorState, u32 PanelIndex)
{
    text_panel *Panel = EditorState->TextPanels + PanelIndex;
    editor_settings *Settings = &EditorState->Settings;

    DrawQuad(OffscreenBitmap,
             Panel->PositionX, Panel->PositionY, Panel->Width, Panel->Height,
             Settings->BackgroundColor);
}

internal void
DrawTextPanel(bitmap *OffscreenBitmap, editor_state *EditorState, u32 PanelIndex)
{
    text_panel *Panel = EditorState->TextPanels + PanelIndex;
    editor_settings *Settings = &EditorState->Settings;

    ClearTextPanelStatusBar(OffscreenBitmap, EditorState, PanelIndex);
    ClearTextPanelBuffer(OffscreenBitmap, EditorState, PanelIndex);

    font *Font = GetFontFromID(&EditorState->FontTable, FontID_Default);
    u32 TitleTextHeight = Font->Ascent + Font->Descent;
    u32 TitleTextPadding = (Settings->StatusBarHeight - TitleTextHeight) / 2;

    if (Panel->FilePath)
    {
        Assert(Panel->CaretCount == 1);
        caret *Caret = Panel->Carets;

        char Whitespace[8] = {};
        u32 WhitespaceCount = 0;

        u32 LineCount = Panel->LineCount + 1;
        u32 LineIndex = Caret->Line + 1;
        while (LineCount > 0)
        {
            if (LineIndex > 0)
            {
                LineIndex /= 10;
            }
            else
            {
                ++WhitespaceCount;
            }

            LineCount /= 10;
        }

        SetMemory(Whitespace, ' ', WhitespaceCount);
        char TitleBuffer[256] = {};
        int Count = sprintf_s(TitleBuffer, sizeof(TitleBuffer), "%s - L#%d%s C#%d",
                              Panel->FilePath, Caret->Line + 1, Whitespace, Caret->Column + 1);

        DrawTextLine(OffscreenBitmap, EditorState,
                     TitleBuffer, Count, FontID_Default,
                     Panel->PositionX + 8, Panel->PositionY - Settings->StatusBarHeight + TitleTextPadding,
                     Settings->TitleColor, UINT32_MAX);
    }
}

internal void
DrawEditorLayout(bitmap *OffscreenBitmap, editor_state *EditorState)
{
    editor_settings *Settings = &EditorState->Settings;
    u32 WindowWidth = EditorState->WindowWidth;
    u32 WindowHeight = EditorState->WindowHeight;

    switch (EditorState->EditorLayout)
    {
        case EditorLayout_Dual:
        {
            text_panel *LPanel = EditorState->TextPanels + 0;
            text_panel *RPanel = EditorState->TextPanels + 1;

            // NOTE(traian): The layout is invalid. Somehow.
            Assert(LPanel->Width + Settings->SeparatorThickness + RPanel->Width == WindowWidth);

            DrawTextPanel(OffscreenBitmap, EditorState, 0);
            DrawTextPanel(OffscreenBitmap, EditorState, 1);

            DrawQuad(OffscreenBitmap, LPanel->Width, Settings->StatusBarHeight,
                     Settings->SeparatorThickness, WindowHeight - Settings->StatusBarHeight, Settings->SeparatorColor);
        } break;
    }
}

void
UpdateAndRenderEditor(editor_state *EditorState, editor_memory *EditorMemory,
                      bitmap *OffscreenBitmap)
{
    DrawEditorLayout(OffscreenBitmap, EditorState);
}

void
ResizeEditor(editor_state *EditorState, editor_memory *EditorMemory, u32 NewWidth, u32 NewHeight)
{
    EditorState->WindowWidth = NewWidth;
    EditorState->WindowHeight = NewHeight;

    ResetEditorLayout(EditorState, EditorMemory, EditorState->EditorLayout);
}
