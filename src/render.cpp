// Copyright (c) 2026 Traian Avram. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause.

internal usize
GetFormatBytesPerPixel(bitmap_format Format)
{
    switch (Format)
    {
        case BitmapFormat_BGRA_8888: return 4;
        case BitmapFormat_R_8:       return 1;
        default:                     return 0;
    }
}

internal usize
GetBytesPerPixel(bitmap* Bitmap)
{
    bitmap_format Format = Bitmap->Format;
    usize Result = GetFormatBytesPerPixel(Format);
    return Result;
}

internal usize
GetBytesPerRow(bitmap* Bitmap)
{
    usize BytesPerPixel = GetBytesPerPixel(Bitmap);
    usize Result = cast(usize, Bitmap->Width) * BytesPerPixel;
    return Result;
}

internal rect2s
GetBitmapRect(bitmap* Bitmap)
{
    rect2s Result = {};
    Result.MaxPoint.X = Bitmap ? Bitmap->Width  : 0;
    Result.MaxPoint.Y = Bitmap ? Bitmap->Height : 0;
    return Result;
}

internal void*
GetPixelAddress(bitmap* Bitmap, u32 OffsetX, u32 OffsetY)
{
    if (OffsetX >= Bitmap->Width) return NULL;
    if (OffsetY >= Bitmap->Height) return NULL;

    usize PixelIndex = cast(usize, OffsetY) * cast(usize, Bitmap->Width)
                     + cast(usize, OffsetX);
    usize BytesPerPixel = GetFormatBytesPerPixel(Bitmap->Format);

    void* Address = Bitmap->Data + (PixelIndex * BytesPerPixel);
    return Address;
}

internal render_group_page*
AllocateGroupPage(arena* Arena, usize Capacity)
{
    render_group_page* Page = PushStruct(Arena, render_group_page);
    Page->Entries = PushArray(Arena, draw_entry, Capacity);
    Page->Capacity = Capacity;
    Page->Count = 0;
    return Page;
}

internal draw_entry*
PushRenderGroupEntry(render_group* Group, arena* Arena)
{
    if (Group->LastPage == NULL)
    {
        render_group_page* NewPage = AllocateGroupPage(Arena, 128);
        Group->FirstPage = NewPage;
        Group->LastPage = NewPage;
    }
    else if (Group->LastPage->Count >= Group->LastPage->Capacity)
    {
        render_group_page* NewPage = AllocateGroupPage(Arena, 128);
        Group->LastPage->Next = NewPage;
        NewPage->Prev = Group->LastPage;
        Group->LastPage = NewPage;
    }

    draw_entry* Entry = &Group->LastPage->Entries[Group->LastPage->Count];
    Group->LastPage->Count++;
    return Entry;
}

internal void
PushQuad(render_group* Group, arena* Arena, rect2s Region, linear_color Color)
{
    draw_entry* Entry = PushRenderGroupEntry(Group, Arena);
    Entry->Type        = DrawEntryType_Quad;
    Entry->Quad.Region = Region;
    Entry->Quad.Color  = Color;
}

internal void 
PushCircle(render_group* Group, arena* Arena, rect2s Region, linear_color Color,
           v2f MinUV, v2f MaxUV, f32 InnerRadius)
{
    draw_entry* Entry = PushRenderGroupEntry(Group, Arena);
    Entry->Type                = DrawEntryType_Circle;
    Entry->Circle.Region       = Region;
    Entry->Circle.Color        = Color;
    Entry->Circle.MinUV        = MinUV;
    Entry->Circle.MaxUV        = MaxUV;
    Entry->Circle.InnerRadius  = InnerRadius;
}

internal void
PushTextGrid(render_group* Group, arena* Arena, text_grid* Grid, font* Font,
             v2s Offset, rect2s ClippingMask)
{
    draw_entry* Entry = PushRenderGroupEntry(Group, Arena);
    Entry->Type                  = DrawEntryType_TextGrid;
    Entry->TextGrid.Grid         = *Grid;
    Entry->TextGrid.Font         = Font;
    Entry->TextGrid.Offset       = Offset;
    Entry->TextGrid.ClippingMask = ClippingMask;
}


internal void
PushBorder(render_group* Group, arena* Arena, rect2s Region, linear_color Color,
           u32 BorderLeft, u32 BorderRight, u32 BorderBottom, u32 BorderTop)
{
    v2u RegionSize = GetSize(Region);
    BorderLeft     = Clamp<u32>(BorderLeft,   0, RegionSize.X);
    BorderBottom   = Clamp<u32>(BorderBottom, 0, RegionSize.Y);
    BorderRight    = Clamp<u32>(BorderRight,  0, RegionSize.X - BorderLeft);
    BorderTop      = Clamp<u32>(BorderTop,    0, RegionSize.Y - BorderBottom);

    // +-------------+
    // |      T      |
    // +---+-----+---+
    // |   |     |   |
    // | L |     | R |
    // |   |     |   |
    // +---+-----+---+
    // |      B      |
    // +-------------+

    rect2s L = {};
    L.MinPoint.X = Region.MinPoint.X;
    L.MinPoint.Y = Region.MinPoint.Y + BorderBottom;
    L.MaxPoint.X = Region.MinPoint.X + BorderLeft;
    L.MaxPoint.Y = Region.MaxPoint.Y - BorderTop;

    rect2s R = {};
    R.MinPoint.X = Region.MaxPoint.X - BorderRight;
    R.MinPoint.Y = Region.MinPoint.Y + BorderBottom;
    R.MaxPoint.X = Region.MaxPoint.X;
    R.MaxPoint.Y = Region.MaxPoint.Y - BorderTop;

    rect2s B = {};
    B.MinPoint = Region.MinPoint;
    B.MaxPoint.X = Region.MaxPoint.X;
    B.MaxPoint.Y = Region.MinPoint.Y + BorderBottom;

    rect2s T = {};
    T.MinPoint.X = Region.MinPoint.X;
    T.MinPoint.Y = Region.MaxPoint.Y - BorderTop;
    T.MaxPoint = Region.MaxPoint;

    PushQuad(Group, Arena, L, Color);
    PushQuad(Group, Arena, R, Color);
    PushQuad(Group, Arena, B, Color);
    PushQuad(Group, Arena, T, Color);
}

struct rounded_quad_corners
{
    rect2s BL;
    rect2s BR;
    rect2s TL;
    rect2s TR;
};

internal rounded_quad_corners
CalculateCornerRegions(rect2s Region, u32 CornerRadius)
{
    //      +---+
    //   TL |   | TR
    // +----+   +----+
    // |             |
    // +----+   +----+
    //   BL |   | BR
    //      +---+

    rounded_quad_corners Result = {};
    
    Result.BL.MinPoint = Region.MinPoint;
    Result.BL.MaxPoint.X = Region.MinPoint.X + CornerRadius;
    Result.BL.MaxPoint.Y = Region.MinPoint.Y + CornerRadius;

    Result.BR.MinPoint.X = Region.MaxPoint.X - CornerRadius;
    Result.BR.MinPoint.Y = Region.MinPoint.Y;
    Result.BR.MaxPoint.X = Region.MaxPoint.X;
    Result.BR.MaxPoint.Y = Region.MinPoint.Y + CornerRadius;

    Result.TL.MinPoint.X = Region.MinPoint.X;
    Result.TL.MinPoint.Y = Region.MaxPoint.Y - CornerRadius;
    Result.TL.MaxPoint.X = Region.MinPoint.X + CornerRadius;
    Result.TL.MaxPoint.Y = Region.MaxPoint.Y;

    Result.TR.MinPoint.X = Region.MaxPoint.X - CornerRadius;
    Result.TR.MinPoint.Y = Region.MaxPoint.Y - CornerRadius;
    Result.TR.MaxPoint = Region.MaxPoint;

    return Result;
}

/*
internal void
PushRoundedBorder(render_group* Group, arena* Arena, rect2s Region,
                  linear_color Color, u32 BorderSize, f32 CornerFallout)
{
    v2u RegionSize = GetSize(Region);
    BorderSize = Clamp<u32>(BorderSize, 0, (RegionSize.X + 1) / 2);
    BorderSize = Clamp<u32>(BorderSize, 0, (RegionSize.Y + 1) / 2);

    //      +-----+
    //   TL |  T  | TR
    // +----+-----+----+
    // |    |     |    |
    // | L  |     |  R |
    // |    |     |    |
    // +----+-----+----+
    //   BL |  B  | BR
    //      +-----+

    rounded_quad_corners Corners = CalculateCornerRegions(Region, BorderSize);

    rect2s B = {};
    B.MinPoint.X = Region.MinPoint.X + BorderSize;
    B.MinPoint.Y = Region.MinPoint.Y;
    B.MaxPoint.X = Region.MaxPoint.X - BorderSize;
    B.MaxPoint.Y = Region.MinPoint.Y + BorderSize;

    rect2s T = {};
    T.MinPoint.X = Region.MinPoint.X + BorderSize;
    T.MinPoint.Y = Region.MaxPoint.Y - BorderSize;
    T.MaxPoint.X = Region.MaxPoint.X - BorderSize;
    T.MaxPoint.Y = Region.MaxPoint.Y;

    rect2s L = {};
    L.MinPoint.X = Region.MinPoint.X;
    L.MinPoint.Y = Region.MinPoint.Y + BorderSize;
    L.MaxPoint.X = Region.MinPoint.X + BorderSize;
    L.MaxPoint.Y = Region.MaxPoint.Y - BorderSize;

    rect2s R = {};
    R.MinPoint.X = Region.MaxPoint.X - BorderSize;
    R.MinPoint.Y = Region.MinPoint.Y + BorderSize;
    R.MaxPoint.X = Region.MaxPoint.X;
    R.MaxPoint.Y = Region.MaxPoint.Y - BorderSize;

    PushQuad(Group, Arena, B, Color);
    PushQuad(Group, Arena, T, Color);
    PushQuad(Group, Arena, L, Color);
    PushQuad(Group, Arena, R, Color);
}
*/

internal void
PushRoundedQuad(render_group* Group, arena* Arena, rect2s Region,
                linear_color Color, u32 CornerRadius, f32 CornerFallout)
{
    v2u RegionSize = GetSize(Region);
    CornerRadius = Clamp<u32>(CornerRadius, 0, (RegionSize.X + 1) / 2);
    CornerRadius = Clamp<u32>(CornerRadius, 0, (RegionSize.Y + 1) / 2);

    //      +-----+
    //   TL |     | TR
    // +----+     +----+
    // |    |     |    |
    // | L  |  M  |  R |
    // |    |     |    |
    // +----+     +----+
    //   BL |     | BR
    //      +-----+

    rect2s L = {};
    L.MinPoint.X = Region.MinPoint.X;
    L.MinPoint.Y = Region.MinPoint.Y + CornerRadius;
    L.MaxPoint.X = Region.MinPoint.X + CornerRadius;
    L.MaxPoint.Y = Region.MaxPoint.Y - CornerRadius;

    rect2s M = {};
    M.MinPoint.X = Region.MinPoint.X + CornerRadius;
    M.MinPoint.Y = Region.MinPoint.Y;
    M.MaxPoint.X = Region.MaxPoint.X - CornerRadius;
    M.MaxPoint.Y = Region.MaxPoint.Y;

    rect2s R = {};
    R.MinPoint.X = Region.MaxPoint.X - CornerRadius;
    R.MinPoint.Y = Region.MinPoint.Y + CornerRadius;
    R.MaxPoint.X = Region.MaxPoint.X;
    R.MaxPoint.Y = Region.MaxPoint.Y - CornerRadius;

    PushQuad(Group, Arena, L, Color);
    PushQuad(Group, Arena, M, Color);
    PushQuad(Group, Arena, R, Color);

    rounded_quad_corners Corners = CalculateCornerRegions(Region, CornerRadius);

    PushCircle(Group, Arena, Corners.BL, Color,
               V2f(-1.0F, -1.0F), V2f(0.0f, 0.0f), 0.0F);
    
    PushCircle(Group, Arena, Corners.BR, Color,
               V2f(0.0F, -1.0F), V2f(1.0f, 0.0f), 0.0F);
    
    PushCircle(Group, Arena, Corners.TL, Color,
               V2f(-1.0F, 0.0F), V2f(0.0f, 1.0f), 0.0F);
    
    PushCircle(Group, Arena, Corners.TR, Color,
               V2f(0.0F, 0.0F), V2f(1.0f, 1.0f), 0.0F);
}

internal void
BlitQuad(bitmap* RenderTarget, rect2s Region, linear_color Color)
{
    //
    // It is the responsability of the render group to clip the quad region
    // such that it does not overflow the visible range (the bitmap dimensions).
    //                                      -- avrtraian 24 May 2026
    ASSERT(0 <= Region.MinPoint.X && Region.MaxPoint.X <= RenderTarget->Width);
    ASSERT(0 <= Region.MinPoint.Y && Region.MaxPoint.Y <= RenderTarget->Height);

    //
    // @Robustness: This is currently the only supported render target format.
    // If in the future there will be more formats supported, this assert will
    // quickly tell us that the implementation is incomplete!
    //                                      -- avrtraian 24 May 2026.
    ASSERT(RenderTarget->Format == BitmapFormat_BGRA_8888);
    u32 PackedColor = PackLinearToBGRA(Color);

    u32* CurrentRow = cast(u32*, GetPixelAddress(RenderTarget,
                                                 Region.MinPoint.X,
                                                 Region.MinPoint.Y));

    for (u32 Y = Region.MinPoint.Y; Y < Region.MaxPoint.Y; ++Y)
    {
        u32* PixelIterator = CurrentRow;
        for (u32 X = Region.MinPoint.X; X < Region.MaxPoint.X; ++X)
        {
            *PixelIterator = PackedColor;
            ++PixelIterator;
        }
        CurrentRow += RenderTarget->Width;
    }
}

internal FORCEINLINE f32
GetCircleSample(v2f SampleUV, f32 InnerRadiusSquared)
{
    f32 DistanceSquared = Dot(SampleUV, SampleUV);
    f32 Result = 0.0F;
    if (InnerRadiusSquared <= DistanceSquared && DistanceSquared <= 1.0F)
    {
        Result = 1.0F;
    }
    return Result;
}

internal void
BlitCircle(bitmap* RenderTarget, rect2s Region, linear_color Color, v2f MinUV,
           v2f MaxUV, f32 InnerRadius)
{
    //
    // It is the responsability of the render group to clip the circle region
    // such that it does not overflow the visible range (the bitmap dimensions)
    // and ajust the min/max UV coordinates accordingly.
    //                                      -- avrtraian 24 May 2026
    ASSERT(0 <= Region.MinPoint.X && Region.MaxPoint.X <= RenderTarget->Width);
    ASSERT(0 <= Region.MinPoint.Y && Region.MaxPoint.Y <= RenderTarget->Height);

    v2f PixelDeltaUV = {};
    PixelDeltaUV.X = (MaxUV.X - MinUV.X) / GetWidth(Region);
    PixelDeltaUV.Y = (MaxUV.Y - MinUV.Y) / GetHeight(Region);
    v2f PixelUV = MinUV + 0.5 * PixelDeltaUV;
    f32 InnerRadiusSquared = InnerRadius * InnerRadius;

    //
    // Implement aliasing using a multi-sampling algorithm. We have 9 samples
    // per pixel, position symetrical and equidistant. For each sample we
    // check whether or not it is inside the circle. The pixel opacity is
    // calculated using a weighted sum.
    // +-------+-------+-------+
    // | 0.050 | 0.125 | 0.050 |
    // +-------+-------+-------+
    // | 0.125 | 0.200 | 0.125 |
    // +-------+-------+-------+
    // | 0.050 | 0.125 | 0.050 |
    // +-------+-------+-------+
    //                                          -- avrtraian 27 May 2026
    //

    v2f SampleOffset0 = V2f(-0.66F * PixelDeltaUV.X, -0.66F * PixelDeltaUV.Y);
    v2f SampleOffset1 = V2f( 0.00F * PixelDeltaUV.X, -0.66F * PixelDeltaUV.Y);
    v2f SampleOffset2 = V2f( 0.66F * PixelDeltaUV.X, -0.66F * PixelDeltaUV.Y);

    v2f SampleOffset3 = V2f(-0.66F * PixelDeltaUV.X,  0.00F * PixelDeltaUV.Y);
    v2f SampleOffset4 = V2f( 0.00F * PixelDeltaUV.X,  0.00F * PixelDeltaUV.Y);
    v2f SampleOffset5 = V2f( 0.66F * PixelDeltaUV.X,  0.00F * PixelDeltaUV.Y);

    v2f SampleOffset6 = V2f(-0.66F * PixelDeltaUV.X,  0.66F * PixelDeltaUV.Y);
    v2f SampleOffset7 = V2f( 0.00F * PixelDeltaUV.X,  0.66F * PixelDeltaUV.Y);
    v2f SampleOffset8 = V2f( 0.66F * PixelDeltaUV.X,  0.66F * PixelDeltaUV.Y);

    f32 SampleWeight0 = 0.2F / 4.0F;
    f32 SampleWeight1 = 0.6F / 4.0F;
    f32 SampleWeight2 = 0.2F / 4.0F;
    f32 SampleWeight3 = 0.6F / 4.0F;
    f32 SampleWeight4 = 0.2F / 1.0F;
    f32 SampleWeight5 = 0.6F / 4.0F;
    f32 SampleWeight6 = 0.2F / 4.0F;
    f32 SampleWeight7 = 0.6F / 4.0F;
    f32 SampleWeight8 = 0.2F / 4.0F;

#define FOR_EACH_SAMPLE(_X) \
    _X(0)                   \
    _X(1)                   \
    _X(2)                   \
    _X(3)                   \
    _X(4)                   \
    _X(5)                   \
    _X(6)                   \
    _X(7)                   \
    _X(8)  

#define ADD_SAMPLE(Index)                                                       \
    v2f SampleUV##Index = PixelUV + SampleOffset##Index;                        \
    f32 Sample##Index = GetCircleSample(SampleUV##Index, InnerRadiusSquared);   \
    Opacity += Sample##Index * SampleWeight##Index;

    //
    // @Robustness: This is currently the only supported render target format.
    // If in the future there will be more formats supported, this assert will
    // quickly tell us that the implementation is incomplete!
    //                                      -- avrtraian 27 May 2026.
    ASSERT(RenderTarget->Format == BitmapFormat_BGRA_8888);
    u32* CurrentRow = cast(u32*, GetPixelAddress(RenderTarget,
                                                 Region.MinPoint.X,
                                                 Region.MinPoint.Y));

    for (u32 Y = Region.MinPoint.Y; Y < Region.MaxPoint.Y; ++Y)
    {
        u32* PixelIterator = CurrentRow;
        for (u32 X = Region.MinPoint.X; X < Region.MaxPoint.X; ++X)
        {        
            f32 Opacity = 0.0F;
            FOR_EACH_SAMPLE(ADD_SAMPLE);

            *PixelIterator = PackLinearToBGRA(LinearColor(Color.R * Opacity,
                                                          Color.G * Opacity,
                                                          Color.B * Opacity));
            PixelUV.X += PixelDeltaUV.X;
            ++PixelIterator;
        }

        PixelUV.X = MinUV.X + 0.5 * PixelDeltaUV.X;
        PixelUV.Y += PixelDeltaUV.Y;
        CurrentRow += RenderTarget->Width;
    }
}

internal void
ExecuteRenderGroup(render_group* Group, bitmap* RenderTarget, rect2s Viewport)
{
    rect2s ClippingMask = Intersect(GetBitmapRect(RenderTarget), Viewport);
    if (IsDegenerated(ClippingMask))
    {
        // Nothing will be visible anyway.
        return;
    }

    for (render_group_page* GroupPage = Group->FirstPage;
         GroupPage != NULL;
         GroupPage = GroupPage->Next)
    {
        for (usize EntryIndex = 0; EntryIndex < GroupPage->Count; ++EntryIndex)
        {
            draw_entry* Entry = &GroupPage->Entries[EntryIndex];
            switch (Entry->Type)
            {
                case DrawEntryType_Quad:
                {
                    draw_entry_quad* Quad = &Entry->Quad;
                    rect2s Region = Intersect(Quad->Region, ClippingMask);
                    BlitQuad(RenderTarget, Region, Quad->Color);
                } break;

                case DrawEntryType_Circle:
                {
                    draw_entry_circle* Circle = &Entry->Circle;
                    rect2s Region = Intersect(Circle->Region, ClippingMask);

                    v2f MinUV = {};
                    v2f MaxUV = {};

                    MinUV.X = LinearMap(Circle->Region.MinPoint.X,
                                        Circle->Region.MaxPoint.X,
                                        Region.MinPoint.X,
                                        Circle->MinUV.X,
                                        Circle->MaxUV.X);

                    MinUV.Y = LinearMap(Circle->Region.MinPoint.Y,
                                        Circle->Region.MaxPoint.Y,
                                        Region.MinPoint.Y,
                                        Circle->MinUV.Y,
                                        Circle->MaxUV.Y);

                    MaxUV.X = LinearMap(Circle->Region.MinPoint.X,
                                        Circle->Region.MaxPoint.X,
                                        Region.MaxPoint.X,
                                        Circle->MinUV.X,
                                        Circle->MaxUV.X);

                    MaxUV.Y = LinearMap(Circle->Region.MinPoint.Y,
                                        Circle->Region.MaxPoint.Y,
                                        Region.MaxPoint.Y,
                                        Circle->MinUV.Y,
                                        Circle->MaxUV.Y);

                    BlitCircle(RenderTarget, Region, Circle->Color, MinUV,
                               MaxUV, Circle->InnerRadius);
                } break;
            }
        }
    }
}
