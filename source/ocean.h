/*  =====================================================================
    $File:   ocean.h $
    $Date:   September 17 2023 $
    $Author: Traian Avram $
    $Notice: Copyright (c) 2023-2023 Traian Avram. All Rights Reserved. $
    =====================================================================  */
#ifndef OCEAN_H

#define OCEAN_VERSION_MAJOR 1
#define OCEAN_VERSION_MINOR 0
#define OCEAN_VERSION_PATCH 0
#define OCEAN_BUILD_DATE    __TIMESTAMP__

#define internal static
#define global static
#define local_persist static

#ifndef OCEAN_DEBUG
    #define OCEAN_DEBUG 0
#endif // OCEAN_DEBUG

#ifndef OCEAN_COMPILER_MSVC
    #define OCEAN_COMPILER_MSVC 0
#endif // OCEAN_COMPILER_MSVC

#if OCEAN_DEBUG
    #define Assert(Expression) if (!(Expression)) { __debugbreak(); }
    #define InvalidCodePath __debugbreak()
#else
    #define Assert(...)
    #define InvalidCodePath
#endif // OCEAN_DEBUG

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))
#define OffsetOf(Type, Member) (u64)(&(((Type *)0)->Member))

#define Kilobytes(X) (1024 * (X))
#define Megabytes(X) Kilobytes(1024 * (X))
#define Gigabytes(X) Megabytes(1024 * (X))

#define Bit(X) (1 << (X))

#define INVALID_SIZE ((memory_size)(-1))

#define Minimum(X, Y) ((X) < (Y) ? (X) : (Y))
#define Maximum(X, Y) ((X) > (Y) ? (X) : (Y))

#include "ocean_math.h"

void CopyMem(void *Destination, void *Source, memory_size Size);
void SetMemory(void *Destination, u8 Value, memory_size Size);
void SetMemoryToZero(void *Destination, memory_size Size);

#define CopyArray(Destination, Source, Count) \
    CopyMem(Destination, Source, (Count) * sizeof((Source)[0]))

struct memory_arena
{
    u8 *Base;
    memory_size Size;
    memory_offset Offset;
};

void InitializeArena(memory_arena *Arena, void *Memory, memory_size Size);
u8 * AllocateFromArena(memory_arena *Arena, memory_size Size);
void ResetArena(memory_arena *Arena);

struct temporary_arena
{
    memory_arena *ParentArena;
    memory_arena Arena;
};

temporary_arena BeginTemporaryArena(memory_arena *Arena);
void EndTemporaryArena(temporary_arena *TemporaryArena);

#define PushSize(Arena, Size) AllocateFromArena((Arena), (Size))
#define PushStruct(Arena, Type) ((Type *)AllocateFromArena((Arena), sizeof(Type)))
#define PushArray(Arena, Type, Count) ((Type *)AllocateFromArena((Arena), (Count) * sizeof(Type)))

inline memory_size
StringLength(char *String)
{
    char *Start = String;
    while (*String++);
    return (String - Start) - 1;
}

struct buffer
{
    u8 *Data;
    memory_size Size;
};

struct editor_memory
{
    memory_size PermanentStorageSize;
    void *PermanentStorage;
    memory_arena PermanentArena;
};

struct bitmap
{
    u32 Width;
    u32 Height;
    u32 Pitch;
    u32 BytesPerPixel;
    u8 *Memory;
};

// NOTE(traian): The number of contiguous ASCII characters that require
// a font glyph. This also represents the number of entries in the
// fast font bitmap cache.
#define FONT_ASCII_COUNT ((u32)('~' - '!' + 1))

// NOTE(traian): Subtract this number from an ASCII character in order to
// obtain the index in the font cache of the glyph.
#define FONT_ASCII_OFFSET ((u32)'!')

struct font_entry
{
    bitmap Bitmap;
    s32 OffsetX;
    s32 OffsetY;
};

struct font
{
    float Height;
    u32 Ascent;
    u32 Descent;
    u32 LineGap;
    // NOTE(traian): Because the editor only supports monospaced fonts,
    // there is no point in storing the same value in every font entry.
    u32 Advance;
    font_entry ASCIIEntries[FONT_ASCII_COUNT];
};

typedef enum font_id_enum
{
    FontID_Default = 0,
    FontID_Text,
    FontID_Interface,

    FontID_MaxCount,
}
font_id;

struct font_table
{
    u32 FontCount;
    font *Fonts;
    // NOTE(traian): Maps a font id to an index in the Fonts array.
    u32 IndexMap[FontID_MaxCount];
};

struct editor_settings
{
    u32 TextColor;
    u32 BackgroundColor;
    u32 LineHighlightColor;
    u32 CaretColor;

    u32 StatusBarColor;
    u32 StatusBarTextColor;

    u32 SeparatorColor;

    u32 CaretWidth;
    u32 StatusBarHeight;
    u32 SeparatorThickness;

    u32 TextPaddingX;
    u32 TextPaddingY;

    u32 TabWidth;
    b32 ReplaceTabWithSpaces;
};

typedef enum editor_layout_enum
{
    EditorLayout_Single,
    EditorLayout_Dual,
    EditorLayout_TripleLeft,
    EditorLayout_TripleRight,
    EditorLayout_Quad,
}
editor_layout;

struct text_buffer
{
    char *Base;
    memory_size Size;
    memory_size Used;
};

struct text_caret_position
{
    memory_size Offset;
    u32 Line;
    u32 Column;
};

struct text_caret
{
    text_caret_position Position;
    u32 TargetColumn;

    text_caret_position Selection;
    b32 IsSelecting;
};

struct text_panel
{
    rectangle2 Surface;
    text_buffer Buffer;
    b32 IsSaveDirty;
    char *FileName;
    u32 LineCount;

    u32 FirstLineIndex;
    u32 FirstColumnIndex;
    memory_size BufferOffset;

    text_caret Caret;

    // NOTE(traian): These are the number of lines/columns that fit completely on the screen.
    // There might be an aditional line at the bottom of the screen that only fits partially. In this
    // case, that line will be handled specially. Same thing goes for the right-most column.
    u32 ScreenLineCount;
    u32 ScreenColumnCount;
};

typedef enum key_modifier_enum : u8
{
    KeyModifier_None  = 0,
    KeyModifier_Ctrl  = Bit(0),
    KeyModifier_Shift = Bit(1),
    KeyModifier_Alt   = Bit(2),
}
key_modifier;

typedef enum key_code_enum : u8
{
    KeyCode_FKeyFirst        = 0,
    KeyCode_FKeyLast         = 11,

    KeyCode_ArrowFirst       = 12,
    KeyCode_ArrowLeft        = 12,
    KeyCode_ArrowRight       = 13,
    KeyCode_ArrowUp          = 14,
    KeyCode_ArrowDown        = 15,
    KeyCode_ArrowLast        = 15,
    
    KeyCode_PageUp,
    KeyCode_PageDown,
    KeyCode_Backspace,
    KeyCode_Delete,

    KeyCode_AlphabetKeyFirst = 'A',
    KeyCode_AlphabetKeyLast  = 'Z',

    KeyCode_Zero, KeyCode_One, KeyCode_Two,   KeyCode_Three, KeyCode_Four,
    KeyCode_Five, KeyCode_Six, KeyCode_Seven, KeyCode_Eight, KeyCode_Nine,

    KeyCode_Space,
    KeyCode_Enter,
    KeyCode_Tab,

    KeyCode_Semicolon,
    KeyCode_Comma,
    KeyCode_Period,
    KeyCode_Equals,
    KeyCode_Minus,
    KeyCode_Apostrophe,
    KeyCode_Backtick,
    KeyCode_Slash,
    KeyCode_Backslash,
    KeyCode_LeftBracket,
    KeyCode_RightBracket,

    KeyCode_Invalid          = 0XFF,
}
key_code;

typedef enum mouse_button_enum : u8
{
    MouseButton_Left,
    MouseButton_Middle,
    MouseButton_Right,
    MouseButton_Next,
    MouseButton_Previous,

    MouseButton_Count,
}
mouse_button;

struct editor_command_info
{
    key_code KeyCode;
    key_modifier Modifiers;
    b32 IsCapsLockActive;

    void *OpaqueData;
};

#define EDITOR_COMMAND(Name) void(Name)(struct editor_state *EditorState, u32 PanelIndex, editor_command_info *CommandInfo)
typedef EDITOR_COMMAND(editor_command_function);

struct command_entry
{
    u32 NextIndex;
    key_modifier Modifiers;
    editor_command_function *Callback;
};

struct command_table
{
    u32 KeyCommands[256];
    u32 MouseCommands[MouseButton_Count];

    command_entry CommandEntriesPool[512];
    u32 CommandEntriesPoolOffset;
};

struct editor_state
{
    // NOTE(traian): These are the dimensions of the window client area.
    u32 WindowWidth;
    u32 WindowHeight;

    editor_settings Settings;
    font_table FontTable;

    editor_layout EditorLayout;
    text_panel TextPanels[4];
    u32 FocusedTextPanelIndex;

    command_table CommandTable;
};

void InitializeEditor(editor_state *EditorState, editor_memory *EditorMemory);
void UpdateAndRenderEditor(editor_state *EditorState, editor_memory *EditorMemory,
                           bitmap *OffscreenBitmap);

void EditorEventKeyPressed(editor_state *EditorState, key_code KeyCode);
void EditorEventKeyReleased(editor_state *EditorState, key_code KeyCode);
void EditorEventMouseButtonPressed(editor_state *EditorState, mouse_button Button);
void EditorEventMouseButtonReleased(editor_state *EditorState, mouse_button Button);
void EditorEventMouseMoved(editor_state *EditorState, s32 OffsetX, s32 OffsetY);
void EditorEventWindowResized(editor_state *EditorState, u32 Width, u32 Height);

//
// NOTE(traian): PLATFORM LAYER.
//

buffer PlatformAllocateMemory(memory_size Size);
void PlatformReleaseMemory(buffer Block);

memory_size PlatformGetFileSize(char *FileName);

memory_size PlatformReadEntireFile(char *FileName, buffer FileBuffer);
buffer PlatformReadEntireFile(char *FileName, memory_arena *Arena);

memory_size PlatformWriteEntireFile(char *FileName, buffer Buffer);

key_modifier PlatformGetKeyModifiers();
b32 PlatformIsCapsLockActive();

void PlatformQuit();
void PlatformToggleFullscreen();

#define OCEAN_H
#endif // OCEAN_H
