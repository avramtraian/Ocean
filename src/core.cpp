// Copyright (c) 2026 Traian Avram. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause.

internal void
HandleAssertionFailed()
{
    // @Incomplete: Print information about the source code location of where
    // the assert was triggered. We should also extract and print the
    // program stack trace... -- avrtraian 27 May 2026
    DEBUGBREAK;
}

internal void
MemoryCopy(void* Destination, void* Source, usize Size)
{
    // @Performance: Replace this naive implementation with intrinsics!
    u8* Dst = cast(u8*, Destination);
    u8* Src = cast(u8*, Source);
    for (usize Offset = 0; Offset < Size; ++Offset)
    {
        Dst[Offset] = Src[Offset];
    }
}

internal void
MemorySet(void* Destination, u8 ByteValue, usize Size)
{
    // @Performance: Replace this naive implementation with intrinsics!
    u8* Dst = cast(u8*, Destination);
    for (usize Offset = 0; Offset < Size; ++Offset)
    {
        Dst[Offset] = ByteValue;
    }
}

internal void
MemoryZero(void* Destination, usize Size)
{
    // @Performance: Replace this naive implementation with intrinsics!
    u8* Dst = cast(u8*, Destination);
    for (usize Offset = 0; Offset < Size; ++Offset)
    {
        Dst[Offset] = 0;
    }
}

internal b8
ArenaInitialize(arena* Arena, usize Reserved, usize Committed)
{
    if (Arena == NULL)
        return false;
    ZERO_STRUCT(Arena);
    
    if (Reserved < Committed)
        return false;
    Reserved = OSGetMemoryPageAligned(Reserved);
    Committed = OSGetMemoryPageAligned(Committed);

    void* ReservedAddress = OSVirtualAllocate(NULL, Reserved, OSAllocateType_Reserve);
    if (ReservedAddress == NULL)
        return false;

    void* CommittedAddress = OSVirtualAllocate(ReservedAddress, Reserved, OSAllocateType_Commit);
    if (CommittedAddress == NULL)
    {
        OSVirtualFree(ReservedAddress, 0, OSFreeType_Release);
        return false;
    }

    Arena->Data = cast(u8*, CommittedAddress);
    Arena->Reserved = Reserved;
    Arena->Committed = Committed;
    Arena->Used = 0;
    return true;
}

internal void
ArenaReset(arena* Arena)
{
    if (Arena == NULL)
        return;
    Arena->Used = 0;
}

internal void*
ArenaAllocate(arena* Arena, usize AllocationSize, usize AllocationAlignment)
{
    if (Arena == NULL || AllocationSize == 0)
        return NULL;

    uintptr BaseAddress = cast(uintptr, Arena->Data + Arena->Used);
    uintptr AlignedAddress = RoundUpPowOfTwo<uintptr>(BaseAddress, AllocationAlignment);
    usize AlignmentOffset = AlignedAddress - BaseAddress;
    usize TotalSize = AlignmentOffset + AllocationSize;

    if (Arena->Used + TotalSize > Arena->Reserved)
        return NULL;

    if (Arena->Used + TotalSize > Arena->Committed)
    {
        usize NewCommitted = Max(2 * Arena->Committed, Arena->Used + TotalSize);
        NewCommitted = OSGetMemoryPageAligned(NewCommitted);

        void* Address = OSVirtualAllocate(Arena->Data + Arena->Committed,
                                          NewCommitted - Arena->Committed,
                                          OSAllocateType_Commit);
        if (Address == NULL)
            return NULL;
        Arena->Committed = NewCommitted;
    }

    void* Allocation = cast(void*, AlignedAddress);
    MemoryZero(Allocation, AllocationSize);
    Arena->Used += TotalSize;
    return Allocation;
}

internal string8
MakeString8(void* Data, usize Size)
{
    string8 Result = {};
    Result.Data = cast(u8*, Data);
    Result.Size = Size;
    return Result;
}
