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
    #define OC_DEBUGBREAK   __debugbreak()
    #define OC_INLINE       __forceinline
#elif OC_COMPILER_CLANG_GCC
    #define OC_DEBUGBREAK   __builtin_trap()
    #define OC_INLINE       inline
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

#define internal        static
#define persistent      static

typedef int8_t      int8;
typedef int16_t     int16;
typedef int32_t     int32;
typedef int64_t     int64;

typedef uint8_t     uint8;
typedef uint16_t    uint16;
typedef uint32_t    uint32;
typedef uint64_t    uint64;

typedef float       float32;
typedef double      float64;

typedef int8_t      bool8;
typedef int32_t     bool32;

#define Assert(CONDITION) if (!(CONDITION)) { OC_DEBUGBREAK; }
#define InvalidCodePath OC_DEBUGBREAK

inline uint32 SafeTruncateU64ToU32(uint64 Value)
{
    Assert(Value <= 0xFFFFFFFF);
    uint32 Result = (uint64)(Value);
    return Result;
}

inline uint16 SafeTruncateU64ToU16(uint64 Value)
{
    Assert(Value <= 0xFFFF);
    uint16 Result = (uint16)(Value);
    return Result;
}

inline uint8 SafeTruncateU64ToU8(uint64 Value)
{
    Assert(Value <= 0xFF);
    uint8 Result = (uint8)(Value);
    return Result;
}

#define Kilobytes(X) (X * 1024ULL)
#define Megabytes(X) (Kilobytes(X) * 1024ULL)
#define Gigabytes(X) (Megabytes(X) * 1024ULL)

#define Bit(X) (1 << (X))
#define ArrayCount(ARRAY) (sizeof(ARRAY) / sizeof(ARRAY[0]))
#define CopyStringToBuffer(STRING, BUFFER) MemoryCopy(BUFFER, STRING, sizeof(STRING))

void MemoryCopy(void* Destination, const void* Source, uint64 Size);
void MemorySet(void* Destination, uint8 Value, uint64 Size);
void MemoryZero(void* Destination, uint64 Size);

struct FVec2
{
    float32 X, Y;
};

struct FVec3
{
    float32 X, Y, Z;
};

struct FVec4
{
    float32 X, Y, Z, W;
};

OC_INLINE FVec2 operator+(const FVec2& A, const FVec2& B)
{
    FVec2 Result = {};
    Result.X = A.X + B.X;
    Result.Y = A.Y + B.Y;
    return Result;
}

OC_INLINE FVec2 operator-(const FVec2& A, const FVec2& B)
{
    FVec2 Result = {};
    Result.X = A.X - B.X;
    Result.Y = A.Y - B.Y;
    return Result;
}

OC_INLINE FVec2 operator*(const FVec2& V, float32 Scalar)
{
    FVec2 Result = {};
    Result.X = V.X * Scalar;
    Result.Y = V.Y * Scalar;
    return Result;
}

OC_INLINE FVec2 operator*(float32 Scalar, const FVec2& V)
{
    FVec2 Result = {};
    Result.X = V.X * Scalar;
    Result.Y = V.Y * Scalar;
    return Result;
}

struct FGameCreateData
{
    char    WindowTitle[256];
    uint32  WindowWidth;
    uint32  WindowHeight;
    int32   WindowPositionX;
    int32   WindowPositionY;
};

void OnCreate(FGameCreateData* GameCreateData);

void OnUpdate(float DeltaTime);

void OnDestroy();

void SetBackgroundColor(FVec4 Color);
void DrawRect(int32 X, int32 Y, int32 Width, int32 Height, int32 Thickness, FVec4 Color);
void DrawFilledRect(int32 X, int32 Y, int32 Width, int32 Height, FVec4 Color);

#define OC_IMPLEMENTATION 1
#if OC_IMPLEMENTATION

#include <stdlib.h>

#if OC_PLATFORM_WINDOWS
#include <Windows.h>
#include <gl/gl.h>
#endif // OC_PLATFORM_WINDOWS

void MemoryCopy(void* Destination, const void* Source, uint64 Size)
{
    memcpy(Destination, Source, (size_t)Size);
}

void MemorySet(void* Destination, uint8 Value, uint64 Size)
{
    memset(Destination, (int)Value, Size);
}

void MemoryZero(void* Destination, uint64 Size)
{
    MemorySet(Destination, 0, Size);
}

struct FLinearMemoryArena
{
    void*   BaseAddress;
    uint64  AllocatedOffset;
    uint64  BlockSize;
};

internal bool8 CreateLinearMemoryArena(FLinearMemoryArena** MemoryArena, uint64 ArenaSize)
{
    if (MemoryArena == NULL)
    {
        return false;
    }
    if (ArenaSize == 0)
    {
        return false;
    }

    uint64 MemoryRequirement = sizeof(FLinearMemoryArena) + ArenaSize;
    void* Memory = malloc(MemoryRequirement);
    if (Memory == NULL)
    {
        return false;
    }
    MemoryZero(Memory, MemoryRequirement);

    *MemoryArena = (FLinearMemoryArena*)Memory;
    FLinearMemoryArena* Arena = *MemoryArena;

    Arena->BaseAddress = ((uint8*)Memory) + sizeof(FLinearMemoryArena);
    Arena->AllocatedOffset = 0;
    Arena->BlockSize = ArenaSize;

    return true;
}

internal void DestroyLinearMemoryArena(FLinearMemoryArena** MemoryArena)
{
    if (MemoryArena == NULL || *MemoryArena == NULL)
    {
        return;
    }

    free((*MemoryArena)->BaseAddress);
    *MemoryArena = NULL;
}

internal void* AllocateLinearMemoryArena(FLinearMemoryArena* MemoryArena, uint64 BlockSize)
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

    void* Memory = ((uint8*)MemoryArena->BaseAddress) + MemoryArena->AllocatedOffset;
    MemoryArena->AllocatedOffset += BlockSize;
    return Memory;
}

internal void ResetLinearMemoryArena(FLinearMemoryArena* MemoryArena)
{
    if (MemoryArena == NULL)
    {
        return;
    }

    MemoryArena->AllocatedOffset = 0;
}

struct FSystemTime
{
    uint16  Year;
    uint8   Month;
    uint8   DayOfWeek;
    uint8   Day;
    uint8   Hour;
    uint8   Minute;
    uint8   Second;
    uint16  Millisecond;
};

internal void PlatformGetSystemTime(FSystemTime* SystemTime)
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

internal uint64 PlatformGetTimeNano()
{
#if OC_PLATFORM_WINDOWS
    LARGE_INTEGER PC;
    QueryPerformanceCounter(&PC);
    uint64 PerformanceCounter = PC.QuadPart;

    LARGE_INTEGER PF;
    QueryPerformanceFrequency(&PF);
    uint64 PerformanceFrequency = PF.QuadPart;

    return (uint64)(((float64)(PerformanceCounter) * 1e9) / (float64)(PerformanceFrequency));
#endif
}

internal uint64 PlatformGetTimeMicro()
{
#if OC_PLATFORM_WINDOWS
    LARGE_INTEGER PC;
    QueryPerformanceCounter(&PC);
    uint64 PerformanceCounter = PC.QuadPart;

    LARGE_INTEGER PF;
    QueryPerformanceFrequency(&PF);
    uint64 PerformanceFrequency = PF.QuadPart;

    return (uint64)(((float64)(PerformanceCounter) * 1e6) / (float64)(PerformanceFrequency));
#endif
}

internal uint64 PlatformGetTimeMilli()
{
#if OC_PLATFORM_WINDOWS
    LARGE_INTEGER PC;
    QueryPerformanceCounter(&PC);
    uint64 PerformanceCounter = PC.QuadPart;

    LARGE_INTEGER PF;
    QueryPerformanceFrequency(&PF);
    uint64 PerformanceFrequency = PF.QuadPart;

    return (PerformanceCounter * 1000) / PerformanceFrequency;
#endif
}

internal uint64 PlatformGetTimeSeconds()
{
#if OC_PLATFORM_WINDOWS
    LARGE_INTEGER PC;
    QueryPerformanceCounter(&PC);
    uint64 PerformanceCounter = PC.QuadPart;

    LARGE_INTEGER PF;
    QueryPerformanceFrequency(&PF);
    uint64 PerformanceFrequency = PF.QuadPart;

    return PerformanceCounter / PerformanceFrequency;
#endif
}

struct FWindowData
{
    char    Title[256];
    uint32  Width;
    uint32  Height;
    int32   PositionX;
    int32   PositionY;
    void*   Handle;
};

struct FGameData
{
    bool32          bIsRunning;
    FWindowData*    WindowData;
};
internal FGameData* GameData;

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

internal FWindowData* CreateGameWindow(FGameCreateData* GameCreateData, FLinearMemoryArena* MemoryArena)
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

    FWindowData* WindowData = (FWindowData*)AllocateLinearMemoryArena(MemoryArena, sizeof(FWindowData));
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

internal void PumpWindowMessages(FWindowData* WindowData)
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
typedef intptr_t GLintptr;

typedef void (APIENTRY *DEBUGPROC)(GLenum source,
            GLenum type,
            GLuint id,
            GLenum severity,
            GLsizei length,
            const GLchar *message,
            const void *userParam);

#define OC_DECLARE_OPENGL_FUNCTION(NAME, RETURN, ...)   \
    typedef RETURN(*PFN_##NAME)(__VA_ARGS__);           \
    internal PFN_##NAME PFN_gl##NAME;

#define OC_LOAD_OPENGL_FUNCTION(NAME)                           \
    PFN_gl##NAME = (PFN_##NAME)wglGetProcAddress("gl"#NAME);    \
    if (PFN_gl##NAME == NULL)                                   \
    {                                                           \
        return false;                                           \
    }

#if OC_PLATFORM_WINDOWS
typedef BOOL(*PFN_SwapIntervalEXT)(int);
internal PFN_SwapIntervalEXT PFN_wglSwapIntervalEXT;
#define wglSwapIntervalEXT PFN_wglSwapIntervalEXT
#endif // OC_PLATFORM_WINDOWS

OC_DECLARE_OPENGL_FUNCTION(GenBuffers, void, GLsizei, GLuint*)
#define glGenBuffers PFN_glGenBuffers

OC_DECLARE_OPENGL_FUNCTION(BindBuffer, void, GLenum, GLuint)
#define glBindBuffer PFN_glBindBuffer

OC_DECLARE_OPENGL_FUNCTION(BufferData, void, GLenum, GLsizeiptr, const void*, GLenum)
#define glBufferData PFN_glBufferData

OC_DECLARE_OPENGL_FUNCTION(BufferSubData, void, GLenum, GLintptr, GLsizeiptr, const void*)
#define glBufferSubData PFN_glBufferSubData

OC_DECLARE_OPENGL_FUNCTION(EnableVertexAttribArray, void, GLuint)
#define glEnableVertexAttribArray PFN_glEnableVertexAttribArray

OC_DECLARE_OPENGL_FUNCTION(VertexAttribPointer, void, GLuint, GLint, GLenum, GLboolean, GLsizei, const void*)
#define glVertexAttribPointer PFN_glVertexAttribPointer

OC_DECLARE_OPENGL_FUNCTION(CreateProgram, GLuint)
#define glCreateProgram PFN_glCreateProgram

OC_DECLARE_OPENGL_FUNCTION(CreateShader, GLuint, GLenum)
#define glCreateShader PFN_glCreateShader

OC_DECLARE_OPENGL_FUNCTION(ShaderSource, void, GLuint, GLsizei, const GLchar**, const GLint*)
#define glShaderSource PFN_glShaderSource

OC_DECLARE_OPENGL_FUNCTION(CompileShader, void, GLuint)
#define glCompileShader PFN_glCompileShader

OC_DECLARE_OPENGL_FUNCTION(AttachShader, void, GLuint, GLuint)
#define glAttachShader PFN_glAttachShader

OC_DECLARE_OPENGL_FUNCTION(LinkProgram, void, GLuint)
#define glLinkProgram PFN_glLinkProgram

OC_DECLARE_OPENGL_FUNCTION(DetachShader, void, GLuint, GLuint)
#define glDetachShader PFN_glDetachShader

OC_DECLARE_OPENGL_FUNCTION(DeleteShader, void, GLuint)
#define glDeleteShader PFN_glDeleteShader

OC_DECLARE_OPENGL_FUNCTION(UseProgram, void, GLuint)
#define glUseProgram PFN_glUseProgram

OC_DECLARE_OPENGL_FUNCTION(GetShaderiv, void, GLuint, GLenum, GLint*)
#define glGetShaderiv PFN_glGetShaderiv

OC_DECLARE_OPENGL_FUNCTION(GetProgramiv, void, GLuint, GLenum, GLint*)
#define glGetProgramiv PFN_glGetProgramiv

OC_DECLARE_OPENGL_FUNCTION(DebugMessageCallback, void, DEBUGPROC, const void*)
#define glDebugMessageCallback PFN_glDebugMessageCallback

OC_DECLARE_OPENGL_FUNCTION(DebugMessageControl, void, GLenum, GLenum, GLenum, GLsizei, const GLuint*, GLboolean)
#define glDebugMessageControl PFN_glDebugMessageControl

OC_DECLARE_OPENGL_FUNCTION(CreateVertexArrays, void, GLsizei, GLuint*)
#define glCreateVertexArrays PFN_glCreateVertexArrays

OC_DECLARE_OPENGL_FUNCTION(BindVertexArray, void, GLuint)
#define glBindVertexArray PFN_glBindVertexArray

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

typedef GLuint FGLHandle;

struct FOpenGLState
{
    FGLHandle BoundVertexArray;
    FGLHandle BoundVertexBuffer;
    FGLHandle BoundIndexBuffer;
    FGLHandle BoundShader;
};
internal FOpenGLState OpenGLState; 

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

internal bool8 InitializeOpenGL(HDC WindowDC)
{
    OpenGLState.BoundVertexArray = 0;
    OpenGLState.BoundVertexBuffer = 0;
    OpenGLState.BoundIndexBuffer = 0;

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

    PFN_wglSwapIntervalEXT = (PFN_SwapIntervalEXT)wglGetProcAddress("wglSwapIntervalEXT");

    OC_LOAD_OPENGL_FUNCTION(GenBuffers);
    OC_LOAD_OPENGL_FUNCTION(BindBuffer);
    OC_LOAD_OPENGL_FUNCTION(BufferData);
    OC_LOAD_OPENGL_FUNCTION(BufferSubData);
    OC_LOAD_OPENGL_FUNCTION(EnableVertexAttribArray);
    OC_LOAD_OPENGL_FUNCTION(VertexAttribPointer);
    OC_LOAD_OPENGL_FUNCTION(CreateProgram);
    OC_LOAD_OPENGL_FUNCTION(CreateShader);
    OC_LOAD_OPENGL_FUNCTION(ShaderSource);
    OC_LOAD_OPENGL_FUNCTION(CompileShader);
    OC_LOAD_OPENGL_FUNCTION(AttachShader);
    OC_LOAD_OPENGL_FUNCTION(LinkProgram);
    OC_LOAD_OPENGL_FUNCTION(DetachShader);
    OC_LOAD_OPENGL_FUNCTION(DeleteShader);
    OC_LOAD_OPENGL_FUNCTION(UseProgram);
    OC_LOAD_OPENGL_FUNCTION(GetShaderiv);
    OC_LOAD_OPENGL_FUNCTION(GetProgramiv);
    OC_LOAD_OPENGL_FUNCTION(DebugMessageCallback);
    OC_LOAD_OPENGL_FUNCTION(DebugMessageControl);
    OC_LOAD_OPENGL_FUNCTION(CreateVertexArrays);
    OC_LOAD_OPENGL_FUNCTION(BindVertexArray);

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

enum EShaderDataTypeEnum
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
};
typedef uint8 EShaderDataType;

struct FVertexBufferElement
{
    EShaderDataType     DataType;
    bool8               bIsNormalized;

    // NOTE(Traian): Reserved for internal use. At the moment, only used
    //   for debugging purposes.
    uint32              Offset;
};

struct FVertexBufferLayout
{
    FVertexBufferElement*  Elements;
    uint32                 ElementsCount;
    uint32                 Stride;
};

enum vertex_buffer_flags_enum
{
    VERTEX_BUFFER_FLAG_NONE             = 0,
    VERTEX_BUFFER_FLAG_DYNAMIC_DRAW_BIT = Bit(0)
};
typedef uint8 EVertexBufferFlags;

enum EIndexBufferTypeEnum
{
    INDEX_BUFFER_TYPE_INVALID   = 0,
    INDEX_BUFFER_TYPE_UINT8     = 1,
    INDEX_BUFFER_TYPE_UINT16    = 2,
    INDEX_BUFFER_TYPE_UINT32    = 3
};
typedef uint8 EIndexBufferType;

struct FVertexArray
{
    FGLHandle               VertexArrayHandle;
    FGLHandle               VertexBufferHandle;
    FVertexBufferLayout     VertexBufferLayout;
    EVertexBufferFlags      VertexBufferFlags;
    FGLHandle               IndexBufferHandle;
    EIndexBufferType        IndexBufferType;
    uint64                  IndexBufferIndicesCount;
};

struct FVertexArraySpecification
{
    FVertexBufferElement*  VertexBufferLayoutElements;
    uint32                 VertexBufferLayoutElementsCount;
    EVertexBufferFlags     VertexBufferFlags;
    const void*            VertexBufferData;
    uint64                 VertexBufferDataSize;
    EIndexBufferType       IndexBufferType;
    const void*            IndexBufferData;
    uint64                 IndexBufferIndicesCount;
};

internal void UTILS_GetOpenGLTypeInfo(EShaderDataType DataType, GLenum* GLType, GLint* GLTypeCount, uint32* GLTypeSize)
{
    GLenum Type;
    GLint TypeCount;
    uint32 TypeSize;

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

internal void RendererBindVertexArray(const FVertexArray* VertexArray)
{
    if (OpenGLState.BoundVertexArray != VertexArray->VertexArrayHandle)
    {
        glBindVertexArray(VertexArray->VertexArrayHandle);
    }
    if (OpenGLState.BoundVertexBuffer != VertexArray->VertexBufferHandle)
    {
        glBindBuffer(GL_ARRAY_BUFFER, VertexArray->VertexBufferHandle);
    }
    if (OpenGLState.BoundIndexBuffer != VertexArray->IndexBufferHandle)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VertexArray->IndexBufferHandle);
    }
}

internal void RendererUnbindVertexArray()
{
    if (OpenGLState.BoundVertexArray)
    {
        glBindVertexArray(0);
    }
    if (OpenGLState.BoundVertexBuffer)
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    if (OpenGLState.BoundIndexBuffer)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}

internal void RendererSetVertexBufferData(FGLHandle VertexBuffer, const void* Data, uint64 DataSize)
{
    if (OpenGLState.BoundVertexBuffer != VertexBuffer)
    {
        glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
    }
    glBufferSubData(GL_ARRAY_BUFFER, 0, DataSize, Data);
}

internal bool8 RendererCreateVertexArray(FVertexArray* VertexArray, const FVertexArraySpecification* Specification, FLinearMemoryArena* MemoryArena)
{
    RendererUnbindVertexArray();

    if (Specification->VertexBufferDataSize == 0)
    {
        // NOTE(Traian): VertexBufferDataSize MUST be > 0.
        Assert(false);
        return false;
    }
    if (Specification->IndexBufferData == NULL || Specification->IndexBufferIndicesCount == 0)
    {
        // NOTE(Traian): The index buffer requires valid data.
        InvalidCodePath;
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
        (FVertexBufferElement*)AllocateLinearMemoryArena(MemoryArena, (uint64)Specification->VertexBufferLayoutElementsCount * sizeof(FVertexBufferElement));
    VertexArray->VertexBufferLayout.ElementsCount = Specification->VertexBufferLayoutElementsCount;

    VertexArray->VertexBufferLayout.Stride = 0;
    for (uint32 Index = 0; Index < Specification->VertexBufferLayoutElementsCount; Index++)
    {
        FVertexBufferElement* BufferElement = VertexArray->VertexBufferLayout.Elements + Index;
        *BufferElement = Specification->VertexBufferLayoutElements[Index];
        BufferElement->Offset = VertexArray->VertexBufferLayout.Stride;

        uint32 GLSize;
        UTILS_GetOpenGLTypeInfo(BufferElement->DataType, NULL, NULL, &GLSize);
        VertexArray->VertexBufferLayout.Stride += GLSize;
    }

    for (uint32 Index = 0; Index < VertexArray->VertexBufferLayout.ElementsCount; Index++)
    {
        FVertexBufferElement* BufferElement = VertexArray->VertexBufferLayout.Elements + Index;
        
        GLenum GLType;
        GLint GLTypeCount;
        UTILS_GetOpenGLTypeInfo(BufferElement->DataType, &GLType, &GLTypeCount, NULL);

        glEnableVertexAttribArray(Index);
        glVertexAttribPointer(
            Index,
            GLTypeCount, GLType, BufferElement->bIsNormalized ? GL_TRUE : GL_FALSE,
            VertexArray->VertexBufferLayout.Stride, (const void*)((uint64)BufferElement->Offset)
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

    return true;
}

internal void RendererDrawIndexedCount(const FVertexArray* VertexArray, uint32 Count)
{
    if (VertexArray == NULL)
    {
        return;
    }

    RendererBindVertexArray(VertexArray);

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

    RendererBindVertexArray(VertexArray);
    glDrawElements(GL_TRIANGLES, Count, GLIndexType, NULL);
}

internal void RendererDrawIndexed(const FVertexArray* VertexArray)
{
    RendererDrawIndexedCount(VertexArray, VertexArray->IndexBufferIndicesCount);
}

struct FQuadVertex
{
    FVec2   Position;
    FVec2   TextureCoords;
    FVec4   Color;
    float32 TextureIndex;
    float32 TilingFactor;
};

#define OC_MAX_QUADS 4096ULL

struct FRendererData
{
    FGLHandle       QuadShader;
    FVertexArray    QuadVertexArray;
    FQuadVertex     QuadVertices[OC_MAX_QUADS * 4];
    uint64          QuadVerticesCount;

#if OC_PLATFORM_WINDOWS
    HDC             DeviceContext;
#endif // OC_PLATFORM_WINDOWS
};
internal FRendererData* RendererData;

internal bool8 InitializeRenderer(FLinearMemoryArena* MemoryArena)
{
    HDC WindowDC = GetDC((HWND)GameData->WindowData->Handle);

    if (!InitializeOpenGL(WindowDC))
    {
        // TODO(Traian): Logging.
        return false;
    }

    RendererData = (FRendererData*)AllocateLinearMemoryArena(MemoryArena, sizeof(FRendererData));
    if (RendererData == NULL)
    {
        return false;
    }

    RendererData->DeviceContext = WindowDC;

    const char* VertexShaderSource = R"(
        #version 450 core
        
        layout(location = 0) in vec2   a_Position;
        layout(location = 1) in vec2   a_TextureCoords;
        layout(location = 2) in vec4   a_Color;
        layout(location = 3) in float  a_TextureIndex;
        layout(location = 4) in float  a_TilingFactor;
        
        layout(location = 0) out vec2  v_TextureCoords;
        layout(location = 1) out vec4  v_Color;
        layout(location = 2) out float v_TextureIndex;
        layout(location = 3) out float v_TilingFactor;
        
        void main()
        {
           v_TextureCoords = a_TextureCoords;
           v_Color         = a_Color;
           v_TextureIndex  = a_TextureIndex;
           v_TilingFactor  = a_TilingFactor;
        
           gl_Position     = vec4(a_Position, 0.0, 1.0);
        }
    )";

    FGLHandle VertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(VertexShader, 1, &VertexShaderSource, NULL);
    glCompileShader(VertexShader);

    GLint VertexSuccess;
    glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &VertexSuccess);
    Assert(VertexSuccess);

    const char* FragmentShaderSource = R"(
        #version 450 core
        
        layout(location = 0) in vec2   v_TextureCoords;
        layout(location = 1) in vec4   v_Color;
        layout(location = 2) in float  v_TextureIndex;
        layout(location = 3) in float  v_TilingFactor;
        
        layout(location = 0) out vec4  o_Color;
        
        void main()
        {
           o_Color = v_Color;
        }
    )";

    FGLHandle FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
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

    FVertexArraySpecification VASpec = {};

    FVertexBufferElement BufferLayoutElements[] = {
        { SHADER_DATA_TYPE_FLOAT_2 },
        { SHADER_DATA_TYPE_FLOAT_2 },
        { SHADER_DATA_TYPE_FLOAT_4 },
        { SHADER_DATA_TYPE_FLOAT_1 },
        { SHADER_DATA_TYPE_FLOAT_1 }
    };

    VASpec.VertexBufferLayoutElements = BufferLayoutElements;
    VASpec.VertexBufferLayoutElementsCount = ArrayCount(BufferLayoutElements);
    VASpec.VertexBufferFlags = VERTEX_BUFFER_FLAG_DYNAMIC_DRAW_BIT;
    VASpec.VertexBufferDataSize = (OC_MAX_QUADS * 4) * sizeof(FQuadVertex);

    // TODO(Traian): Implement a transient memory arena to use here.
    // NOTE(Traian): We use the vertices buffer for the indices. This is possible as long as no
    //   multithreading is performed.
    uint32* IndicesData = (uint32*)RendererData->QuadVertices;
    for (uint64 Index = 0; Index < OC_MAX_QUADS; Index++)
    {
        IndicesData[6 * Index + 0] = 0 + (4 * Index);
        IndicesData[6 * Index + 1] = 1 + (4 * Index);
        IndicesData[6 * Index + 2] = 2 + (4 * Index);
        IndicesData[6 * Index + 3] = 0 + (4 * Index);
        IndicesData[6 * Index + 4] = 2 + (4 * Index);
        IndicesData[6 * Index + 5] = 3 + (4 * Index);
    }

    VASpec.IndexBufferType = INDEX_BUFFER_TYPE_UINT32;
    VASpec.IndexBufferData = IndicesData;
    VASpec.IndexBufferIndicesCount = OC_MAX_QUADS * 6;

    if (!RendererCreateVertexArray(&RendererData->QuadVertexArray, &VASpec, MemoryArena))
    {
        // TODO(Traian): Logging.
        Assert(false);
        return false;
    }

    return true;
}

internal void RendererSwapBuffers()
{
#if OC_PLATFORM_WINDOWS
    SwapBuffers(RendererData->DeviceContext);
#endif // OC_PLATFORM_WINDOWS
}

internal void RendererBeginScene()
{
    RendererData->QuadVerticesCount = 0;
}

internal void RendererEndScene()
{
    if (RendererData->QuadVerticesCount > 0)
    {
        RendererSetVertexBufferData(
            RendererData->QuadVertexArray.VertexBufferHandle,
            RendererData->QuadVertices, RendererData->QuadVerticesCount * sizeof(FQuadVertex));
        RendererDrawIndexedCount(&RendererData->QuadVertexArray, RendererData->QuadVerticesCount * 3 / 2);
        
        RendererData->QuadVerticesCount = 0;
    }
}

struct FQuadSpecification
{
    FVec2   Position;
    FVec2   Size;
    FVec4   Color;
    float32 Rotation;
    float32 TilingFactor;

    // TODO(Traian): Texture handle.
};

internal void RendererSubmitQuad(const FQuadSpecification* QuadSpecification)
{
    FQuadVertex* Vertex = RendererData->QuadVertices + RendererData->QuadVerticesCount;
    Vertex->Position = QuadSpecification->Position;
    Vertex->Color = QuadSpecification->Color;

    Vertex++;
    Vertex->Position = QuadSpecification->Position + FVec2{ 0.0F, QuadSpecification->Size.Y };
    Vertex->Color = QuadSpecification->Color;

    Vertex++;
    Vertex->Position = QuadSpecification->Position + FVec2{ QuadSpecification->Size.X, QuadSpecification->Size.Y };
    Vertex->Color = QuadSpecification->Color;

    Vertex++;
    Vertex->Position = QuadSpecification->Position + FVec2{ QuadSpecification->Size.X, 0.0F };
    Vertex->Color = QuadSpecification->Color;

    RendererData->QuadVerticesCount += 4;
}

internal int32 GuardedMain(char** CommandLineArguments, uint16 CommandLineArgumentsCount)
{
    FGameData GD = {};
    GameData = &GD;

    FGameCreateData GameCreateData = {};
    OnCreate(&GameCreateData);

    FLinearMemoryArena* CoreSystemsMemoryArena;
    CreateLinearMemoryArena(&CoreSystemsMemoryArena, Megabytes(16));

    FWindowData* WindowData = CreateGameWindow(&GameCreateData, CoreSystemsMemoryArena);
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
    glClearColor(0.0F, 0.0F, 0.0F, 1.0F);

    uint64 LastTime = PlatformGetTimeMilli();
    GameData->bIsRunning = true;
    while (GameData->bIsRunning)
    {
        PumpWindowMessages(WindowData);
        
        uint64 CurrentTime = PlatformGetTimeMilli();
        uint64 DeltaTime = CurrentTime - LastTime;
        LastTime = CurrentTime;

        glClear(GL_COLOR_BUFFER_BIT);

        RendererBeginScene();
        OnUpdate((float32)(DeltaTime * 1e-3));
        RendererEndScene();

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

void SetBackgroundColor(FVec4 Color)
{
    glClearColor(Color.X, Color.Y, Color.Z, 1.0F);
}

void DrawRect(int32 X, int32 Y, int32 Width, int32 Height, int32 Thickness, FVec4 Color)
{
    DrawFilledRect(X,                       Y,                      Thickness,          Height - Thickness, Color);
    DrawFilledRect(X,                       Y + Height - Thickness, Width - Thickness,  Thickness,          Color);
    DrawFilledRect(X + Width - Thickness,   Y + Thickness,          Thickness,          Height - Thickness, Color);
    DrawFilledRect(X + Thickness,           Y,                      Width - Thickness,  Thickness,          Color);
}

void DrawFilledRect(int32 X, int32 Y, int32 Width, int32 Height, FVec4 Color)
{
    FQuadSpecification QuadSpec = {};

    float32 OneOverWindowWidth = 1.0F / (float32)GameData->WindowData->Width;
    float32 OneOverWindowHeight = 1.0F / (float32)GameData->WindowData->Height;

    QuadSpec.Position.X = ((float32)X * OneOverWindowWidth)  * 2.0F - 1.0F;
    QuadSpec.Position.Y = ((float32)Y * OneOverWindowHeight) * 2.0F - 1.0F;

    QuadSpec.Size.X = (float32)Width * OneOverWindowWidth * 2.0F;
    QuadSpec.Size.Y = (float32)Height * OneOverWindowHeight * 2.0F;

    QuadSpec.Color = Color;

    RendererSubmitQuad(&QuadSpec);
}

#endif // OC_IMPLEMENTATION