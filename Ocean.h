/**
 *--------------------------------------------------------------------------------------
 * Ocean.h
 *--------------------------------------------------------------------------------------
 * Ocean is a framework that aims towards creating a platform that can
 *   used for creating performance-critical demos & presentations, while
 *   having a simple and descriptive API.
 * 
 * The GitHub project can be found at: 'https://github.com/avramtraian/Ocean'.
 */

/**
 * MIT License
 * 
 * Copyright(c) 2022-2022 Avram Traian
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files(the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions :
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#ifndef OC_IMPLEMENTATION
    #define OC_IMPLEMENTATION                       0
#endif // OC_IMPLEMENTATION

#ifdef _WIN32
    #define OC_PLATFORM_WINDOWS                     1
    #ifdef _WIN64
        #define OC_PLATFORM_WINDOWS_64              1
    #else
        #define OC_PLATFORM_WINDOWS_32              1
    #endif // _WIN64
#endif // _WIN32

#ifndef OC_PLATFORM_WINDOWS
    #define OC_PLATFORM_WINDOWS                     0
#endif // OC_PLATFORM_WINDOWS

#ifndef OC_PLATFORM_WINDOWS_32
    #define OC_PLATFORM_WINDOWS_32                  0
#endif // OC_PLATFORM_WINDOWS_32

#ifndef OC_PLATFORM_WINDOWS_64
    #define OC_PLATFORM_WINDOWS_64                  0
#endif // OC_PLATFORM_WINDOWS_64

#if !OC_PLATFORM_WINDOWS && !OC_PLATFORM_LINUX && !OC_PLATFORM_MACOS
    #error Unknown or unsupported platform! Ocean can only be used on Windows.
#endif

#ifdef _MSC_BUILD
    #define OC_COMPILER_MSVC                        1
#endif // _MSC_BUILD

#ifdef __clang__
    #define OC_COMPILER_CLANG                       1
    #define OC_COMPILER_CLANG_GCC                   1
#endif // __clang__

#ifdef __gcc__
    #define OC_COMPILER_GCC                         1
    #define OC_COMPILER_CLANG_GCC                   1
#endif // __gcc__

#ifndef OC_COMPILER_MSVC
    #define OC_COMPILER_MSVC                        0
#endif // OC_COMPILER_MSVC
#ifndef OC_COMPILER_CLANG
    #define OC_COMPILER_CLANG                       0
#endif // OC_COMPILER_CLANG
#ifndef OC_COMPILER_GCC
    #define OC_COMPILER_GCC                         0
#endif // OC_COMPILER_GCC
#ifndef OC_COMPILER_CLANG_GCC
    #define OC_COMPILER_CLANG_GCC                   0
#endif // OC_COMPILER_CLANG_GCC

#if !OC_COMPILER_MSVC && !OC_COMPILER_CLANG && !OC_COMPILER_GCC
    #error Unknown or unsupported compiler! Ocean can only be compiled with MSVC, Clang or GCC.
#endif

#if OC_COMPILER_MSVC
    #define OC_DEBUGBREAK __debugbreak()
#elif OC_COMPILER_CLANG_GCC
    #define OC_DEBUGBREAK __builtin_trap()
#endif

#define OC_CONFIGURATION_ALREADY_DEFINED            0

#ifdef OC_DEBUG
    #if OC_DEBUG
        #undef OC_CONFIGURATION_ALREADY_DEFINED
        #define OC_CONFIGURATION_ALREADY_DEFINED    1
    #endif // OC_DEBUG
#endif // OC_DEBUG
#ifdef OC_RELEASE
    #if OC_RELEASE
        #undef OC_CONFIGURATION_ALREADY_DEFINED
        #define OC_CONFIGURATION_ALREADY_DEFINED    1
    #endif // OC_RELEASE
#endif // OC_RELEASE

#if !OC_CONFIGURATION_ALREADY_DEFINED
    #ifdef _DEBUG
        #define OC_DEBUG                            1
    #else
        #define OC_RELEASE                          1
    #endif // _DEBUG
#endif // !OC_CONFIGURATION_ALREADY_DEFINED

#undef OC_CONFIGURATION_ALREADY_DEFINED

#ifndef OC_DEBUG
    #define OC_DEBUG                                0
#endif // OC_DEBUG
#ifndef OC_RELEASE
    #define OC_RELEASE                              0
#endif // OC_RELEASE

#if !OC_DEBUG && !OC_RELEASE
    #error No build configuration was specified!
#endif

#include <stdint.h>
#include <string.h>

#define internal    static
#define persistent  static

#define true        (1)
#define false       (0)

typedef int8_t      s8;
typedef int16_t     s16;
typedef int32_t     s32;
typedef int64_t     s64;

typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;

typedef float       r32;
typedef double      r64;

typedef int8_t      b8;
typedef int32_t     b32;

#define Assert(CONDITION) if (!(CONDITION)) { OC_DEBUGBREAK; }

#define Kilobytes(X) (X * 1024ULL)
#define Megabytes(X) (Kilobytes(X) * 1024ULL)
#define Gigabytes(X) (Megabytes(X) * 1024ULL)

#define ArrayCount(ARRAY) (sizeof(ARRAY) / sizeof(ARRAY[0]))
#define SetStringToBuffer(STRING, BUFFER) MemoryCopy(BUFFER, STRING, sizeof(STRING))
#define Bit(X) (1 << (X))

void MemoryCopy(void* Destination, const void* Source, u64 Size);
void MemorySet(void* Destination, u8 Value, u64 Size);
void MemoryZero(void* Destination, u64 Size);

typedef struct v2
{
    r32 X;
    r32 Y;
}
v2;

typedef struct v3
{
    r32 X;
    r32 Y;
    r32 Z;
}
v3;

typedef struct v4
{
    r32 X;
    r32 Y;
    r32 Z;
    r32 W;
}
v4;

typedef struct game_create_data
{
    char    WindowTitle[256];
    u32     WindowWidth;
    u32     WindowHeight;
    s32     WindowPositionX;
    s32     WindowPositionY;
}
game_create_data;

void OnCreate(game_create_data* GameCreateData);

void OnUpdate(float DeltaTime);

void OnDestroy();

#if OC_IMPLEMENTATION

#include <stdlib.h>

#if OC_PLATFORM_WINDOWS
#include <Windows.h>
#include <GL/gl.h>
#endif // OC_PLATFORM_WINDOWS

void MemoryCopy(void* Destination, const void* Source, u64 Size)
{
    memcpy(Destination, Source, (size_t)Size);
}

void MemorySet(void* Destination, u8 Value, u64 Size)
{
    memset(Destination, (int)Value, Size);
}

void MemoryZero(void* Destination, u64 Size)
{
    MemorySet(Destination, 0, Size);
}

typedef struct linear_memory_arena
{
    void*   BaseAddress;
    u64     AllocatedOffset;
    u64     BlockSize;
}
linear_memory_arena;

internal b8 CreateLinearMemoryArena(linear_memory_arena** MemoryArena, u64 ArenaSize)
{
    if (MemoryArena == NULL)
    {
        return false;
    }
    if (ArenaSize == 0)
    {
        return false;
    }

    u64 MemoryRequirement = sizeof(linear_memory_arena) + ArenaSize;
    void* Memory = malloc(MemoryRequirement);
    if (Memory == NULL)
    {
        return false;
    }
    MemoryZero(Memory, MemoryRequirement);

    *MemoryArena = (linear_memory_arena*)Memory;
    linear_memory_arena* Arena = *MemoryArena;

    Arena->BaseAddress = ((u8*)Memory) + sizeof(linear_memory_arena);
    Arena->AllocatedOffset = 0;
    Arena->BlockSize = ArenaSize;

    return true;
}

internal void DestroyLinearMemoryArena(linear_memory_arena** MemoryArena)
{
    if (MemoryArena == NULL || *MemoryArena == NULL)
    {
        return;
    }

    free((*MemoryArena)->BaseAddress);
    *MemoryArena = NULL;
}

internal void* AllocateLinearMemoryArena(linear_memory_arena* MemoryArena, u64 BlockSize)
{
    if (MemoryArena == NULL)
    {
        return NULL;
    }
    if (BlockSize == 0)
    {
        return NULL;
    }

    if (MemoryArena->AllocatedOffset + BlockSize > MemoryArena->BlockSize)
    {
        // TODO(Traian): Logging.
        return NULL;
    }

    void* Memory = ((u8*)MemoryArena->BaseAddress) + MemoryArena->AllocatedOffset;
    MemoryArena->AllocatedOffset += BlockSize;
    return Memory;
}

internal void ResetLinearMemoryArena(linear_memory_arena* MemoryArena)
{
    if (MemoryArena == NULL)
    {
        return;
    }

    MemoryArena->AllocatedOffset = 0;
}

typedef struct system_time
{
    u16 Year;
    u8  Month;
    u8  DayOfWeek;
    u8  Day;
    u8  Hour;
    u8  Minute;
    u8  Second;
    u16 Millisecond;
}
system_time;

internal void PlatformGetSystemTime(system_time* SystemTime)
{
    SYSTEMTIME SysTime;
    GetSystemTime(&SysTime);

    SystemTime->Year = SysTime.wYear;
    SystemTime->Month = SysTime.wMonth;
    SystemTime->DayOfWeek = SysTime.wDayOfWeek;
    SystemTime->Day = SysTime.wDay;
    SystemTime->Hour = SysTime.wHour;
    SystemTime->Minute = SysTime.wMinute;
    SystemTime->Second = SysTime.wSecond;
    SystemTime->Millisecond = SysTime.wMilliseconds;
}

internal u64 PlatformGetTimeNano()
{
#if OC_PLATFORM_WINDOWS
    LARGE_INTEGER PC;
    QueryPerformanceCounter(&PC);
    u64 PerformanceCounter = PC.QuadPart;

    LARGE_INTEGER PF;
    QueryPerformanceFrequency(&PF);
    u64 PerformanceFrequency = PF.QuadPart;

    return (u64)(((r64)(PerformanceCounter) * 1e9) / (r64)(PerformanceFrequency));
#endif
}

internal u64 PlatformGetTimeMicro()
{
#if OC_PLATFORM_WINDOWS
    LARGE_INTEGER PC;
    QueryPerformanceCounter(&PC);
    u64 PerformanceCounter = PC.QuadPart;

    LARGE_INTEGER PF;
    QueryPerformanceFrequency(&PF);
    u64 PerformanceFrequency = PF.QuadPart;

    return (u64)(((r64)(PerformanceCounter) * 1e6) / (r64)(PerformanceFrequency));
#endif
}

internal u64 PlatformGetTimeMilli()
{
#if OC_PLATFORM_WINDOWS
    LARGE_INTEGER PC;
    QueryPerformanceCounter(&PC);
    u64 PerformanceCounter = PC.QuadPart;

    LARGE_INTEGER PF;
    QueryPerformanceFrequency(&PF);
    u64 PerformanceFrequency = PF.QuadPart;

    return (PerformanceCounter * 1000) / PerformanceFrequency;
#endif
}

internal u64 PlatformGetTimeSeconds()
{
#if OC_PLATFORM_WINDOWS
    LARGE_INTEGER PC;
    QueryPerformanceCounter(&PC);
    u64 PerformanceCounter = PC.QuadPart;

    LARGE_INTEGER PF;
    QueryPerformanceFrequency(&PF);
    u64 PerformanceFrequency = PF.QuadPart;

    return PerformanceCounter / PerformanceFrequency;
#endif
}

typedef struct window_data
{
    char    Title[256];
    u32     Width;
    u32     Height;
    s32     PositionX;
    s32     PositionY;
    void*   Handle;
} window_data;

typedef struct game_data
{
    b32             bIsRunning;
    window_data*    WindowData;
}
game_data;
internal game_data* GameData;

internal LRESULT OceanWindowProcedure(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam)
{
    switch (Message)
    {
        case WM_CLOSE:
        {
            GameData->bIsRunning = false;
        }
        return 0;
    }

    return DefWindowProc(WindowHandle, Message, WParam, LParam);
}

internal window_data* CreateGameWindow(game_create_data* GameCreateData, linear_memory_arena* MemoryArena)
{
    if (GameCreateData == NULL || MemoryArena == NULL)
    {
        return NULL;
    }

#if OC_PLATFORM_WINDOWS
    HINSTANCE Instance = GetModuleHandle(NULL);

    WNDCLASSA WindowClass = {};
    WindowClass.lpfnWndProc = OceanWindowProcedure;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = "OceanWindowClass";
    if (!RegisterClassA(&WindowClass))
    {
        // TODO(Traian): Logging.
        return NULL;
    }

    HWND WindowHandle = CreateWindowA(
        "OceanWindowClass",
        GameCreateData->WindowTitle,
        WS_OVERLAPPEDWINDOW,
        (int)GameCreateData->WindowPositionX, (int)GameCreateData->WindowPositionX,
        (int)GameCreateData->WindowWidth, (int)GameCreateData->WindowHeight,
        NULL, NULL, Instance, NULL
    );

    if (WindowHandle == NULL)
    {
        // TODO(Traian): Logging.
        return NULL;
    }

    window_data* WindowData = (window_data*)AllocateLinearMemoryArena(MemoryArena, sizeof(window_data));
    WindowData->Handle = WindowHandle;

    MemoryCopy(WindowData->Title, GameCreateData->WindowTitle, sizeof(WindowData->Title));
    WindowData->Width = GameCreateData->WindowWidth;
    WindowData->Height = GameCreateData->WindowHeight;
    WindowData->PositionX = GameCreateData->WindowPositionX;
    WindowData->PositionY = GameCreateData->WindowPositionY;

    ShowWindow(WindowHandle, SW_SHOW);

    return WindowData;
#endif // OC_PLATFORM_WINDOWS
}

internal void PumpWindowMessages(window_data* WindowData)
{
    MSG Message;
    while (PeekMessageA(&Message, (HWND)WindowData->Handle, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&Message);
        DispatchMessageA(&Message);
    }
}

typedef char GLchar;
typedef ptrdiff_t GLsizeiptr;

typedef void (APIENTRY *DEBUGPROC)(GLenum source,
            GLenum type,
            GLuint id,
            GLenum severity,
            GLsizei length,
            const GLchar *message,
            const void *userParam);

#define OC_DECLARE_OPENGL_FUNCTION(NAME, RETURN, ...)   \
    typedef RETURN(*PFN##NAME)(__VA_ARGS__);            \
    internal PFN##NAME NAME;

#define OC_LOAD_OPENGL_FUNCTION(NAME)                   \
    NAME = (PFN##NAME)wglGetProcAddress(#NAME);         \
    if (NAME == NULL)                                   \
    {                                                   \
        return false;                                   \
    }

#if OC_PLATFORM_WINDOWS
OC_DECLARE_OPENGL_FUNCTION(wglSwapIntervalEXT, BOOL, int)
#endif // OC_PLATFORM_WINDOWS

OC_DECLARE_OPENGL_FUNCTION(glGenBuffers,                void, GLsizei, GLuint*)
OC_DECLARE_OPENGL_FUNCTION(glBindBuffer,                void, GLenum, GLuint)
OC_DECLARE_OPENGL_FUNCTION(glBufferData,                void, GLenum, GLsizeiptr, const void*, GLenum)
OC_DECLARE_OPENGL_FUNCTION(glEnableVertexAttribArray,   void, GLuint)
OC_DECLARE_OPENGL_FUNCTION(glVertexAttribPointer,       void, GLuint, GLint, GLenum, GLboolean, GLsizei, const void*)
OC_DECLARE_OPENGL_FUNCTION(glCreateProgram,             GLuint)
OC_DECLARE_OPENGL_FUNCTION(glCreateShader,              GLuint, GLenum)
OC_DECLARE_OPENGL_FUNCTION(glShaderSource,              void, GLuint, GLsizei, const GLchar**, const GLint*)
OC_DECLARE_OPENGL_FUNCTION(glCompileShader,             void, GLuint)
OC_DECLARE_OPENGL_FUNCTION(glAttachShader,              void, GLuint, GLuint)
OC_DECLARE_OPENGL_FUNCTION(glLinkProgram,               void, GLuint)
OC_DECLARE_OPENGL_FUNCTION(glDetachShader,              void, GLuint, GLuint)
OC_DECLARE_OPENGL_FUNCTION(glDeleteShader,              void, GLuint)
OC_DECLARE_OPENGL_FUNCTION(glUseProgram,                void, GLuint)
OC_DECLARE_OPENGL_FUNCTION(glGetShaderiv,               void, GLuint, GLenum, GLint*)
OC_DECLARE_OPENGL_FUNCTION(glGetProgramiv,              void, GLuint, GLenum, GLint*)
OC_DECLARE_OPENGL_FUNCTION(glDebugMessageCallback,      void, DEBUGPROC, const void*)
OC_DECLARE_OPENGL_FUNCTION(glDebugMessageControl,       void, GLenum, GLenum, GLenum, GLsizei, const GLuint*, GLboolean)
OC_DECLARE_OPENGL_FUNCTION(glCreateVertexArrays,        void, GLsizei, GLuint*)
OC_DECLARE_OPENGL_FUNCTION(glBindVertexArray,           void, GLuint)

#define GL_ARRAY_BUFFER             0x8892
#define GL_ELEMENT_ARRAY_BUFFER     0x8893
#define GL_STATIC_DRAW              0x88E4
#define GL_DYNAMIC_DRAW             0x88E8
#define GL_FRAGMENT_SHADER          0x8B30
#define GL_VERTEX_SHADER            0x8B31
#define GL_COMPILE_STATUS           0x8B81
#define GL_LINK_STATUS              0x8B82
#define GL_CONTEXT_FLAGS            0x821E
#define GL_CONTEXT_FLAG_DEBUG_BIT   0x00000002
#define GL_DEBUG_OUTPUT             0x92E0
#define GL_DEBUG_OUTPUT_SYNCHRONOUS 0x8242

internal void OpenGLDebugCallback(
    GLenum      Source,
    GLenum      Type,
    GLuint      ID,
    GLenum      Severity,
    GLsizei     Length,
    const char* Message,
    const void* UserParam
)
{
    Assert(false);
}

internal b8 InitializeOpenGL(HDC WindowDC)
{
#if OC_PLATFORM_WINDOWS

    PIXELFORMATDESCRIPTOR DesiredPixelFormat = {};
    DesiredPixelFormat.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    DesiredPixelFormat.nVersion = 1;
    DesiredPixelFormat.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
    DesiredPixelFormat.iPixelType = PFD_TYPE_RGBA;
    DesiredPixelFormat.cColorBits = 32;
    DesiredPixelFormat.cAlphaBits = 8;
    DesiredPixelFormat.cDepthBits = 24;
    DesiredPixelFormat.cStencilBits = 8;
    DesiredPixelFormat.iLayerType = PFD_MAIN_PLANE;

    int SuggestedPixelFormatIndex = ChoosePixelFormat(WindowDC, &DesiredPixelFormat);

    PIXELFORMATDESCRIPTOR SuggestedPixelFormat = {};
    DescribePixelFormat(WindowDC, SuggestedPixelFormatIndex, sizeof(PIXELFORMATDESCRIPTOR), &SuggestedPixelFormat);

    SetPixelFormat(WindowDC, SuggestedPixelFormatIndex, &SuggestedPixelFormat);

    HGLRC OpenGLContext = wglCreateContext(WindowDC);
    wglMakeCurrent(WindowDC, OpenGLContext);

    OC_LOAD_OPENGL_FUNCTION(wglSwapIntervalEXT);
    OC_LOAD_OPENGL_FUNCTION(glGenBuffers);
    OC_LOAD_OPENGL_FUNCTION(glBindBuffer);
    OC_LOAD_OPENGL_FUNCTION(glBufferData);
    OC_LOAD_OPENGL_FUNCTION(glEnableVertexAttribArray);
    OC_LOAD_OPENGL_FUNCTION(glVertexAttribPointer);
    OC_LOAD_OPENGL_FUNCTION(glCreateProgram);
    OC_LOAD_OPENGL_FUNCTION(glCreateShader);
    OC_LOAD_OPENGL_FUNCTION(glShaderSource);
    OC_LOAD_OPENGL_FUNCTION(glCompileShader);
    OC_LOAD_OPENGL_FUNCTION(glAttachShader);
    OC_LOAD_OPENGL_FUNCTION(glLinkProgram);
    OC_LOAD_OPENGL_FUNCTION(glDetachShader);
    OC_LOAD_OPENGL_FUNCTION(glDeleteShader);
    OC_LOAD_OPENGL_FUNCTION(glUseProgram);
    OC_LOAD_OPENGL_FUNCTION(glGetShaderiv);
    OC_LOAD_OPENGL_FUNCTION(glGetProgramiv);
    OC_LOAD_OPENGL_FUNCTION(glDebugMessageCallback);
    OC_LOAD_OPENGL_FUNCTION(glDebugMessageControl);
    OC_LOAD_OPENGL_FUNCTION(glCreateVertexArrays);
    OC_LOAD_OPENGL_FUNCTION(glBindVertexArray);

    GLint OpenGLFlags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &OpenGLFlags);
    if (OpenGLFlags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(OpenGLDebugCallback, NULL);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
    }

    wglSwapIntervalEXT(1);
    return true;

#endif // OC_PLATFORM_WINDOWS
}

typedef GLuint gl_handle;

typedef enum shader_data_type_enum
{
    SHADER_DATA_TYPE_INVALID    = 0,

    SHADER_DATA_TYPE_FLOAT_1    = 1,
    SHADER_DATA_TYPE_FLOAT_2    = 2,
    SHADER_DATA_TYPE_FLOAT_3    = 3,
    SHADER_DATA_TYPE_FLOAT_4    = 4,

    // SHADER_DATA_TYPE_INT_1      = 5,
    // SHADER_DATA_TYPE_INT_2      = 6,
    // SHADER_DATA_TYPE_INT_3      = 7,
    // SHADER_DATA_TYPE_INT_4      = 8,

    // SHADER_DATA_TYPE_UINT_1     = 9,
    // SHADER_DATA_TYPE_UINT_2     = 10,
    // SHADER_DATA_TYPE_UINT_3     = 11,
    // SHADER_DATA_TYPE_UINT_4     = 12,

    // SHADER_DATA_TYPE_MAT_3      = 13,
    // SHADER_DATA_TYPE_MAT_4      = 14
}
shader_data_type_enum;
typedef u8 shader_data_type;

typedef struct vertex_buffer_element
{
    shader_data_type    DataType;
    b8                  bIsNormalized;
    // NOTE(Traian): Reserved for internal use. At the moment, only used
    //   for debugging purposes.
    u32                 Offset;
}
vertex_buffer_element;

typedef struct vertex_buffer_layout
{
    vertex_buffer_element*  Elements;
    u32                     ElementsCount;
    u32                     Stride;
}
vertex_buffer_layout;

typedef enum vertex_buffer_flags_enum
{
    VERTEX_BUFFER_FLAG_NONE             = 0,
    VERTEX_BUFFER_FLAG_DYNAMIC_DRAW_BIT = Bit(0)
}
vertex_buffer_flags_enum;
typedef u8 vertex_buffer_flags;

typedef enum index_buffer_type_enum
{
    INDEX_BUFFER_TYPE_INVALID   = 0,
    INDEX_BUFFER_TYPE_UINT8     = 1,
    INDEX_BUFFER_TYPE_UINT16    = 2,
    INDEX_BUFFER_TYPE_UINT32    = 3
}
index_buffer_type_enum;
typedef u8 index_buffer_type;

typedef struct vertex_array
{
    gl_handle               VertexArrayHandle;
    gl_handle               VertexBufferHandle;
    vertex_buffer_layout    VertexBufferLayout;
    vertex_buffer_flags     VertexBufferFlags;
    gl_handle               IndexBufferHandle;
    index_buffer_type       IndexBufferType;
    u64                     IndexBufferIndicesCount;
}
vertex_array;

typedef struct vertex_array_specification
{
    vertex_buffer_element*  VertexBufferLayoutElements;
    u32                     VertexBufferLayoutElementsCount;
    vertex_buffer_flags     VertexBufferFlags;
    const void*             VertexBufferData;
    u64                     VertexBufferDataSize;
    index_buffer_type       IndexBufferType;
    const void*             IndexBufferData;
    u64                     IndexBufferIndicesCount;
}
vertex_array_specification;

internal void UTILSGetOpenGLTypeInfo(shader_data_type DataType, GLenum* GLType, GLint* GLTypeCount, u32* GLTypeSize)
{
    GLenum Type;
    GLint TypeCount;
    u32 TypeSize;

    switch (DataType)
    {
        case SHADER_DATA_TYPE_FLOAT_1:
        {
            Type = GL_FLOAT;
            TypeCount = 1;
            TypeSize = 4;
        }
        break;

        case SHADER_DATA_TYPE_FLOAT_2:
        {
            Type = GL_FLOAT;
            TypeCount = 2;
            TypeSize = 8;
        }
        break;

        case SHADER_DATA_TYPE_FLOAT_3:
        {
            Type = GL_FLOAT;
            TypeCount = 3;
            TypeSize = 12;
        }
        break;

        case SHADER_DATA_TYPE_FLOAT_4:
        {
            Type = GL_FLOAT;
            TypeCount = 4;
            TypeSize = 16;
        }
        break;

        case SHADER_DATA_TYPE_INVALID:
        {
            // NOTE(Traian): Invalid DataType.
            // TODO(Traian): Logging.
            Assert(false);
        }
    }

    if (GLType)
    {
        *GLType = Type;
    }
    if (GLTypeCount)
    {
        *GLTypeCount = TypeCount;
    }
    if (GLTypeSize)
    {
        *GLTypeSize = TypeSize;
    }
}

internal b8 RendererCreateVertexArray(vertex_array* VertexArray, const vertex_array_specification* Specification, linear_memory_arena* MemoryArena)
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    if (Specification->VertexBufferDataSize == 0)
    {
        // NOTE(Traian): VertexBufferDataSize MUST be > 0.
        Assert(false);
        return false;
    }
    if (Specification->IndexBufferData == NULL || Specification->IndexBufferIndicesCount == 0)
    {
        // NOTE(Traian): The index buffer requires valid data.
        Assert(false);
        return false;
    }

    glCreateVertexArrays(1, &VertexArray->VertexArrayHandle);
    glBindVertexArray(VertexArray->VertexArrayHandle);

    glGenBuffers(1, &VertexArray->VertexBufferHandle);
    glBindBuffer(GL_ARRAY_BUFFER, VertexArray->VertexBufferHandle);
    VertexArray->VertexBufferFlags = Specification->VertexBufferFlags;
    GLenum BufferDataFlag = (VertexArray->VertexBufferFlags & VERTEX_BUFFER_FLAG_DYNAMIC_DRAW_BIT) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
    glBufferData(GL_ARRAY_BUFFER, Specification->VertexBufferDataSize, Specification->VertexBufferData, BufferDataFlag);

    VertexArray->VertexBufferLayout.Elements =
        (vertex_buffer_element*)AllocateLinearMemoryArena(MemoryArena, Specification->VertexBufferLayoutElementsCount * sizeof(vertex_buffer_element));
    VertexArray->VertexBufferLayout.ElementsCount = Specification->VertexBufferLayoutElementsCount;

    VertexArray->VertexBufferLayout.Stride = 0;
    for (u32 Index = 0; Index < Specification->VertexBufferLayoutElementsCount; Index++)
    {
        vertex_buffer_element* BufferElement = VertexArray->VertexBufferLayout.Elements + Index;
        *BufferElement = Specification->VertexBufferLayoutElements[Index];
        BufferElement->Offset = VertexArray->VertexBufferLayout.Stride;

        u32 GLSize;
        UTILSGetOpenGLTypeInfo(BufferElement->DataType, NULL, NULL, &GLSize);
        VertexArray->VertexBufferLayout.Stride += GLSize;
    }

    for (u32 Index = 0; Index < VertexArray->VertexBufferLayout.ElementsCount; Index++)
    {
        vertex_buffer_element* BufferElement = VertexArray->VertexBufferLayout.Elements + Index;
        
        GLenum GLType;
        GLint GLTypeCount;
        UTILSGetOpenGLTypeInfo(BufferElement->DataType, &GLType, &GLTypeCount, NULL);

        glEnableVertexAttribArray(Index);
        glVertexAttribPointer(
            Index,
            GLTypeCount, GLType, BufferElement->bIsNormalized ? GL_TRUE : GL_FALSE,
            VertexArray->VertexBufferLayout.Stride, (const void*)((u64)BufferElement->Offset)
        );
    }

    glGenBuffers(1, &VertexArray->IndexBufferHandle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VertexArray->IndexBufferHandle);
    VertexArray->IndexBufferIndicesCount = Specification->IndexBufferIndicesCount;
    VertexArray->IndexBufferType = Specification->IndexBufferType;

    GLsizeiptr TypeSize;
    switch (VertexArray->IndexBufferType)
    {
        case INDEX_BUFFER_TYPE_UINT8:   { TypeSize = 1; break; }
        case INDEX_BUFFER_TYPE_UINT16:  { TypeSize = 2; break; }
        case INDEX_BUFFER_TYPE_UINT32:  { TypeSize = 4; break; }
        case INDEX_BUFFER_TYPE_INVALID:
        {
            // NOTE(Traian): Invalid IndexBufferType.
            // TODO(Traian): Logging.
            Assert(false);
        }
        break;
    }

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, Specification->IndexBufferIndicesCount * TypeSize, Specification->IndexBufferData, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    return true;
}

internal void RendererDraw(const vertex_array* VertexArray)
{
    if (VertexArray == NULL)
    {
        return;
    }

    glBindVertexArray(VertexArray->VertexArrayHandle);
    glBindBuffer(GL_ARRAY_BUFFER, VertexArray->VertexBufferHandle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VertexArray->IndexBufferHandle);

    GLenum GLIndexType;
    switch (VertexArray->IndexBufferType)
    {
        case INDEX_BUFFER_TYPE_UINT8:   { GLIndexType = GL_UNSIGNED_BYTE;   break; }
        case INDEX_BUFFER_TYPE_UINT16:  { GLIndexType = GL_UNSIGNED_SHORT;  break; }
        case INDEX_BUFFER_TYPE_UINT32:  { GLIndexType = GL_UNSIGNED_INT;    break; }
        case INDEX_BUFFER_TYPE_INVALID:
        {
            // NOTE(Traian): Invalid IndexBufferType.
            // TODO(Traian): Logging.
            Assert(false);
        }
        break;
    }

    glDrawElements(GL_TRIANGLES, VertexArray->IndexBufferIndicesCount, GLIndexType, NULL);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

typedef struct quad_vertex
{
    v2  Position;
    v2  TextureCoords;
    v4  Color;
    r32 TextureIndex;
    r32 TilingFactor;
}
quad_vertex;

typedef struct renderer_data
{
    gl_handle       QuadShader;
    vertex_array    QuadVertexArray;

#if OC_PLATFORM_WINDOWS
    HDC             DeviceContext;
#endif // OC_PLATFORM_WINDOWS
}
renderer_data;
internal renderer_data* RendererData;

internal b8 InitializeRenderer(linear_memory_arena* MemoryArena)
{
    HDC WindowDC = GetDC((HWND)GameData->WindowData->Handle);

    if (!InitializeOpenGL(WindowDC))
    {
        // TODO(Traian): Logging.
        return false;
    }

    RendererData = (renderer_data*)AllocateLinearMemoryArena(MemoryArena, sizeof(renderer_data));
    if (RendererData == NULL)
    {
        return false;
    }

    RendererData->DeviceContext = WindowDC;

    const char* VertexShaderSource =
    "#version 450 core\n"
    ""
    "layout(location = 0) in vec2   a_Position;"
    "layout(location = 1) in vec2   a_TextureCoords;"
    "layout(location = 2) in vec4   a_Color;"
    "layout(location = 3) in float  a_TextureIndex;"
    "layout(location = 4) in float  a_TilingFactor;"
    ""
    "layout(location = 0) out vec2  v_TextureCoords;"
    "layout(location = 1) out vec4  v_Color;"
    "layout(location = 2) out float v_TextureIndex;"
    "layout(location = 3) out float v_TilingFactor;"
    ""
    "void main()"
    "{"
    "   v_TextureCoords = a_TextureCoords;"
    "   v_Color         = a_Color;"
    "   v_TextureIndex  = a_TextureIndex;"
    "   v_TilingFactor  = a_TilingFactor;"
    ""
    "   gl_Position     = vec4(a_Position, 0.0, 1.0);"
    "}";

    gl_handle VertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(VertexShader, 1, &VertexShaderSource, NULL);
    glCompileShader(VertexShader);

    GLint VertexSuccess;
    glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &VertexSuccess);
    Assert(VertexSuccess);

    const char* FragmentShaderSource =
    "#version 450 core\n"
    ""
    "layout(location = 0) in vec2   v_TextureCoords;"
    "layout(location = 1) in vec4   v_Color;"
    "layout(location = 2) in float  v_TextureIndex;"
    "layout(location = 3) in float  v_TilingFactor;"
    ""
    "layout(location = 0) out vec4  o_Color;"
    ""
    "void main()"
    "{"
    "   o_Color = v_Color;"
    "}";

    gl_handle FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(FragmentShader, 1, &FragmentShaderSource, NULL);
    glCompileShader(FragmentShader);

    GLint FragmentSuccess;
    glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &FragmentSuccess);
    Assert(FragmentSuccess);

    RendererData->QuadShader = glCreateProgram();
    glAttachShader(RendererData->QuadShader, VertexShader);
    glAttachShader(RendererData->QuadShader, FragmentShader);
    glLinkProgram(RendererData->QuadShader);

    GLint LinkSuccess;
    glGetProgramiv(RendererData->QuadShader, GL_LINK_STATUS, &LinkSuccess);
    Assert(LinkSuccess);

    glDeleteShader(VertexShader);
    glDeleteShader(FragmentShader);

    glUseProgram(RendererData->QuadShader);
    
    quad_vertex QuadVertices[] = {
        { -0.5F, -0.5F, 0.0F, 0.0F, 1.0f, 0.0f, 0.0f, 1.0f, 0.0F, 1.0F },
        {  0.0F,  0.5F, 0.0F, 0.0F, 0.0f, 1.0f, 0.0f, 1.0f, 0.0F, 1.0F },
        {  0.5F, -0.5F, 0.0F, 0.0F, 0.0f, 0.0f, 1.0f, 1.0f, 0.0F, 1.0F }
    };

    u32 IndicesData[] = {
        0, 1, 2
    };

    vertex_array_specification VASpec = {};

    vertex_buffer_element BufferLayoutElements[] = {
        { SHADER_DATA_TYPE_FLOAT_2 },
        { SHADER_DATA_TYPE_FLOAT_2 },
        { SHADER_DATA_TYPE_FLOAT_4 },
        { SHADER_DATA_TYPE_FLOAT_1 },
        { SHADER_DATA_TYPE_FLOAT_1 }
    };

    VASpec.VertexBufferLayoutElements = BufferLayoutElements;
    VASpec.VertexBufferLayoutElementsCount = ArrayCount(BufferLayoutElements);
    VASpec.VertexBufferData = QuadVertices;
    VASpec.VertexBufferDataSize = sizeof(QuadVertices);

    VASpec.IndexBufferType = INDEX_BUFFER_TYPE_UINT32;
    VASpec.IndexBufferData = IndicesData;
    VASpec.IndexBufferIndicesCount = ArrayCount(IndicesData);

    RendererCreateVertexArray(&RendererData->QuadVertexArray, &VASpec, MemoryArena);

    return true;
}

internal void RendererSwapBuffers()
{
#if OC_PLATFORM_WINDOWS
    SwapBuffers(RendererData->DeviceContext);
#endif // OC_PLATFORM_WINDOWS
}

typedef struct quad_specification
{
    v2  Position;
    v2  Size;
    r32 Rotation;
    r32 TilingFactor;
    // TODO: Texture handle.
}
quad_specification;

internal void RendererSubmitQuad(const quad_specification* QuadSpecification)
{

}

internal s32 GuardedMain(char** CommandLineArguments, u16 CommandLineArgumentsCount)
{
    game_data GD = {};
    GameData = &GD;

    game_create_data GameCreateData = {};
    OnCreate(&GameCreateData);

    linear_memory_arena* CoreSystemsMemoryArena;
    CreateLinearMemoryArena(&CoreSystemsMemoryArena, Megabytes(1));

    window_data* WindowData = CreateGameWindow(&GameCreateData, CoreSystemsMemoryArena);
    if (WindowData == NULL)
    {
        // TODO(Traian): Logging.
        return -1;
    }
    GameData->WindowData = WindowData;

    if (!InitializeRenderer(CoreSystemsMemoryArena))
    {
        // TODO(Traian): Logging.
        return -2;
    }

    glViewport(0, 0, WindowData->Width, WindowData->Height);
    glClearColor(0.9F, 0.8F, 0.2F, 1.0F);

    u64 LastTime = PlatformGetTimeMicro();
    GameData->bIsRunning = true;
    while (GameData->bIsRunning)
    {
        PumpWindowMessages(WindowData);
        
        u64 CurrentTime = PlatformGetTimeMicro();
        u64 DeltaTime = CurrentTime - LastTime;
        LastTime = CurrentTime;

        OnUpdate((r32)(DeltaTime * 1e-3));

        glClear(GL_COLOR_BUFFER_BIT);

        RendererDraw(&RendererData->QuadVertexArray);

        RendererSwapBuffers();
    }

    OnDestroy();
    return 0;
}

#if OC_PLATFORM_WINDOWS
int WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCommand)
{
    return (int)(GuardedMain(__argv, __argc));
}
#endif // OC_PLATFORM_WINDOWS

#endif // OC_IMPLEMENTATION