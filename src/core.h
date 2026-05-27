// Copyright (c) 2026 Traian Avram. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause.

#ifndef CORE_H
#define CORE_H

#ifdef _WIN32
    #define PLATFORM_OS_WINDOWS 1
#endif // _WIN32

#ifndef PLATFORM_OS_WINDOWS
    #define PLATFORM_OS_WINDOWS 0
#endif // PLATFORM_OS_WINDOWS

#define _PLATFORM_OS_COUNT PLATFORM_OS_WINDOWS
#if _PLATFORM_OS_COUNT == 0
    #error No platform operating system was detected!
#elif _PLATFORM_OS_COUNT >= 2
    #error Multiple platform operating systems were detected!
#endif
#undef _PLATFORM_OS_COUNT

#if PLATFORM_OS_WINDOWS
    #ifdef _WIN64
        #define PLATFORM_ARCH_X64 1
    #else
        #define PLATFORM_ARCH_X86 1
    #endif // _WIN64
#endif // PLATFORM_OS_WINDOWS

#ifndef PLATFORM_ARCH_X86
    #define PLATFORM_ARCH_X86 0
#endif // PLATFORM_ARCH_X86

#ifndef PLATFORM_ARCH_X64
    #define PLATFORM_ARCH_X64 0
#endif // PLATFORM_ARCH_X64

#define _PLATFORM_ARCH_COUNT PLATFORM_ARCH_X86 + PLATFORM_ARCH_X64
#if _PLATFORM_ARCH_COUNT == 0
    #error No platform architecture was detected!
#elif _PLATFORM_ARCH_COUNT >= 2
    #error Multiple platform architectures were detected!
#endif
#undef _PLATFORM_ARCH_COUNT

#if !defined(__clang__) && defined(_MSC_BUILD)
    #define PLATFORM_COMPILER_MSVC 1
#endif

#ifndef PLATFORM_COMPILER_MSVC
    #define PLATFORM_COMPILER_MSVC 0
#endif // PLATFORM_COMPILER_MSVC

#define _PLATFORM_COMPILER_COUNT PLATFORM_COMPILER_MSVC
#if _PLATFORM_COMPILER_COUNT == 0
    #error No platform compiler was detected!
#elif _PLATFORM_COMPILER_COUNT >= 2
    #error Multiple platform compilers were detected!
#endif
#undef _PLATFORM_COMPILER_COUNT

typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef unsigned long long  u64;

typedef signed char         s8;
typedef signed short        s16;
typedef signed int          s32;
typedef signed long long    s64;

typedef float               f32;
typedef float               f64;

typedef bool                b8;
typedef int                 b32;

#if PLATFORM_ARCH_X86
typedef u32 usize;
typedef s32 ssize;
typedef u32 uintptr;
#endif // 32-bit architectures.

#if PLATFORM_ARCH_X64
typedef u64 usize;
typedef s64 ssize;
typedef u64 uintptr;
#endif // 64-bit architectures.

#define internal            static
#define local_persistent    static
#define global_variable     static
#define NULL                (0)
#define cast(Type, Value)   ((Type)(Value))

#define KiB(X) ((u64)1024 * (X))
#define MiB(X) ((u64)1024 * KiB(X))
#define GiB(X) ((u64)1024 * MiB(X))

#define ASSERT(X)
#define PANIC(...)
#if PLATFORM_COMPILER_MSVC
    #define FORCEINLINE __forceinline
    #define DEBUGBREAK  __debugbreak()
#endif // PLATFORM_COMPILER_MSVC

internal void MemoryCopy(void* Destination, void* Source, usize Size);
internal void MemorySet (void* Destination, u8 ByteValue, usize Size);
internal void MemoryZero(void* Destination, usize Size);

#define COPY_STRUCT(Destination, Source)                            \
    {                                                               \
        static_assert(sizeof(*(Destination)) == sizeof(*(Source))); \
        MemoryCopy(Destination, Source, sizeof(*(Destination)));    \
    }

#define ZERO_STRUCT(X)      MemoryZero(X, sizeof(*(X)))
#define ZERO_ARRAY(X, C)    MemoryZero(X, (C) * sizeof((X)[0]))

struct memory_buffer
{
    u8*   Data;
    usize Size;
};

struct arena
{
    u8*   Data;
    usize Reserved;
    usize Committed;
    usize Used;
};

internal b8 ArenaInitialize(arena* Arena, usize Reserved, usize Committed);

internal void ArenaReset(arena* Arena);

internal void* ArenaAllocate(arena* Arena, usize AllocationSize, usize AllocationAlignment);

#define PushStruct(Arena, StructType) \
    ((StructType*)ArenaAllocate(Arena, sizeof(StructType), alignof(StructType)))

#define PushArray(Arena, ElementType, Count) \
    ((ElementType*)ArenaAllocate(Arena, (Count) * sizeof(ElementType), alignof(ElementType)))

struct string8
{
    u8*   Data;
    usize Size;
};

internal string8 MakeString8(void* Data, usize Size);

#define Str8Lit(X) MakeString8(X, sizeof(X) - sizeof(u8))

//============================================================================//
//---------------------------- DOUBLE LINKED LIST ----------------------------//
//============================================================================//

#define DECLARE_DLL(Name, ElementType) struct Name { DLL_HEADER(ElementType) }
#define DLL_HEADER(ElementType)                             \
    ElementType* Head;                                      \
    ElementType* Tail;

#define DLLForEach(List, It)                                \
    for (auto* It = (List)->Head;                           \
         It != NULL;                                        \
         It = It->Next)

#define DLLAppendTail(List, Node)                           \
    {                                                       \
        (Node)->Prev = (List)->Tail;                        \
        (Node)->Next = NULL;                                \
        if ((List)->Tail == NULL)                           \
        {                                                   \
            (List)->Head = Node;                            \
            (List)->Tail = Node;                            \
        }                                                   \
        else                                                \
        {                                                   \
            (List)->Tail->Next = Node;                      \
            (List)->Tail = Node;                            \
        }                                                   \
    }

#define DLLRemove(List, Node)                               \
    {                                                       \
        auto* _Prev = (Node)->Prev;                         \
        auto* _Next = (Node)->Next;                         \
        (Node)->Prev = NULL;                                \
        (Node)->Next = NULL;                                \
        if (_Prev) _Prev->Next = _Next;                     \
        if (_Next) _Next->Prev = _Prev;                     \
        if ((List)->Head == (Node)) (List)->Head = _Next;   \
        if ((List)->Tail == (Node)) (List)->Tail = _Prev;   \
    }

//============================================================================//
//----------------------------------- QUEUE ----------------------------------//
//============================================================================//

#define DECLARE_QUEUE(Name, ElementType) struct Name { QUEUE_HEADER(ElementType) }
#define QUEUE_HEADER(ElementType)                   \
    ElementType* Head;                              \
    ElementType* Tail;

#define QueueEnqueue(Queue, Node)                   \
    {                                               \
        (Node)->Next = NULL;                        \
        if ((Queue)->Tail == NULL)                  \
        {                                           \
            (Queue)->Head = Node;                   \
            (Queue)->Tail = Node;                   \
        }                                           \
        else                                        \
        {                                           \
            (Queue)->Tail->Next = Node;             \
            (Queue)->Tail = Node;                   \
        }                                           \
    }

#define QueueDequeue(Queue, OutElement)             \
    {                                               \
        OutElement = (Queue)->Head;                 \
        if ((Queue)->Head)                          \
        {                                           \
            (Queue)->Head = (Queue)->Head->Next;    \
            (OutElement)->Next = NULL;              \
        }                                           \
        if ((Queue)->Head == NULL)                  \
        {                                           \
            (Queue)->Tail = NULL;                   \
        }                                           \
    }

//============================================================================//
//----------------------------------- STACK ----------------------------------//
//============================================================================//

#define DECLARE_STACK(Name, ElementType) struct Name { STACK_HEADER(ElementType) }
#define STACK_HEADER(ElementType)                   \
    ElementType* Head;

#define StackPush(Stack, Element)                   \
    {                                               \
        (Element)->Next = (Stack)->Head;            \
        (Stack)->Head = Element;                    \
    }

#define StackPop(Stack, OutElement)                 \
    {                                               \
        OutElement = (Stack)->Head;                 \
        if ((Stack)->Head)                          \
        {                                           \
            (Stack)->Head = (Stack)->Head->Next;    \
            (OutElement)->Next = NULL;              \
        }                                           \
    }

#endif // CORE_H
