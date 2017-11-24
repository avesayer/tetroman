// game.cpp --- 
// 
// Filename: game.cpp
// Author: Sierra
// Created: Вт окт 10 10:32:14 2017 (+0300)
// Last-Updated: Пт окт 27 14:17:15 2017 (+0300)
//           By: Sierra
//

struct vector2
{
    r32 x;
    r32 y;
};

inline static s32
Max2(s32 a, s32 b)
{
    return (a > b) ? a : b;
}

inline static s32
Min2(s32 a, s32 b)
{
    return (a < b) ? a : b;
}

inline static s32 
Min3(s32 a, s32 b, s32 c)
{
    return fmin(fmin(a,b),c);
}

static void
PrintArray1D(u32 *Array, u32 Size)
{
    for (u32 i = 0; i < Size; ++i)
    {
        printf("%d ", Array[i]);
    }
    
    printf("\n");
}

static void
PrintArray2D(s32 **Array, u32 RowAmount, u32 ColumnAmount)
{
    for (u32 i = 0; i < RowAmount; ++i) {
        for (u32 j = 0; j < ColumnAmount; ++j) {
            printf("%d ", Array[i][j]);
        }
        printf("\n");
    }
    
}


enum figure_form
{
    O_figure, I_figure, L_figure, J_figure,
    Z_figure, S_figure, T_figure
};

enum figure_type
{
    classic, stone, mirror
};

struct figure_unit
{
    bool IsStick;
    bool IsEnlarged; // not sure if we need it
    bool IsIdle;
    
    u32 Index;     
    r32 Angle;
    r32 DefaultAngle;
    SDL_RendererFlip Flip;
    
    game_point Center;
    game_point DefaultCenter;
    game_point Shell[4];
    game_point DefaultShell[4];
    game_rect AreaQuad;
    
    figure_form Form;
    figure_type Type;
    
    game_texture *Texture;
};


struct figure_entity
{
    u32 FigureAmountReserved;
    u32 ReturnIndex;
    s32 FigureActive;
    u32 FigureAmount;
    u32 *FigureOrder;
    figure_unit *FigureUnit;
    
    game_rect FigureArea;
    
    bool IsGrabbed;
    bool IsRotating;
    bool IsFlipping;
    bool IsReturning;
    bool IsRestarting;
    
    r32 AreaAlpha;
    r32 FigureAlpha;
    
    r32 FadeInSum;
    r32 FadeOutSum;
    r32 RotationSum;
};

struct sticked_unit
{
    s32 Index;
    u32 Row[4];
    u32 Col[4];
    
    bool IsSticked;
    game_point Center;
};

struct moving_block
{
    game_rect AreaQuad;
    //game_texture *Texture;
    
    u32 RowNumber;
    u32 ColNumber;
    
    bool IsMoving;
    bool IsVertical;
    bool MoveSwitch;
};

struct grid_entity
{
    u32 RowAmount;
    u32 ColumnAmount;
    
    u32 MovingBlocksAmount;
    u32 MovingBlocksAmountReserved;
    
    u32 StickUnitsAmount;
    game_rect GridArea;
    
    r32 **UnitSize;
    s32 **UnitField;
    
    sticked_unit *StickUnits;
    moving_block *MovingBlocks;
    
    game_texture *NormalSquareTexture;
    game_texture *VerticalSquareTexture;
    game_texture *HorizontlaSquareTexture;
};

struct level_entity
{
    grid_entity   *GridEntity;
    figure_entity *FigureEntity;
    
    u32 ActiveBlockSize;
    u32 InActiveBlockSize;
    
    r32 RotationVel;
    r32 StartAlphaPerSec;
    r32 FlippingAlphaPerSec;
    r32 GridScalePerSec;
    
    bool LevelStarted;
    bool LevelPaused;
    bool LevelFinished;
};

struct level_editor
{
    game_rect GridButtonLayer;
    game_rect GridButtonQuad[6];
    
    game_rect FigureButtonLayer;
    game_rect FigureButtonQuad[6];
    
    game_texture *PlusTexture;
    game_texture *MinusTexture;
    game_texture *RowTexture;
    game_texture *ColumnTexture;
    game_texture *RotateTexture;
    game_texture *FlipTexture;
    game_texture *FormTexture;
    game_texture *TypeTexture;
    
    game_font *Font;
};


#include "game.h"
#include "asset_game.h"

static void
DEBUGRenderQuad(game_offscreen_buffer *Buffer, game_rect *AreaQuad, SDL_Color color, u8 Alpha)
{
    u8 r, g, b, a;
    SDL_GetRenderDrawColor(Buffer->Renderer, &r, &g, &b, &a);
    
    SDL_SetRenderDrawColor(Buffer->Renderer, color.r, color.g, color.b, Alpha);
    SDL_RenderDrawRect(Buffer->Renderer, AreaQuad);
    SDL_SetRenderDrawColor(Buffer->Renderer, r, g, b, a);
}

static void
DEBUGRenderQuadFill(game_offscreen_buffer *Buffer, game_rect *AreaQuad, SDL_Color color, u8 Alpha)
{
    u8 r, g, b, a;
    SDL_GetRenderDrawColor(Buffer->Renderer, &r, &g, &b, &a);
    
    SDL_SetRenderDrawColor(Buffer->Renderer, color.r, color.g, color.b, Alpha);
    SDL_RenderFillRect(Buffer->Renderer, AreaQuad);
    SDL_SetRenderDrawColor(Buffer->Renderer, r, g, b, a);
}

static void
DEBUGRenderFigureShell(game_offscreen_buffer *Buffer, figure_unit *Entity, u32 BlockSize, SDL_Color color)
{
    u8 r, g, b;
    SDL_GetRenderDrawColor(Buffer->Renderer, &r, &g, &b, 0);
    SDL_SetRenderDrawColor(Buffer->Renderer, color.r, color.g, color.b, 255);
    
    game_rect Rect = {};
    
    for (u32 i = 0; i < 4; ++i)
    {
        Rect.w = BlockSize;
        Rect.h = BlockSize;
        Rect.x = Entity->Shell[i].x - (Rect.w / 2);
        Rect.y = Entity->Shell[i].y - (Rect.h / 2);
        
        SDL_RenderFillRect(Buffer->Renderer, &Rect);
    }
    
    //SDL_SetRenderDrawColor(Buffer->Renderer, 255, 255, 255, 255);
    Rect.x = Entity->Center.x - (Rect.w / 2);
    Rect.y = Entity->Center.y - (Rect.h / 2);
    //SDL_RenderDrawRect(Buffer->Renderer, &Rect);
    
    SDL_SetRenderDrawColor(Buffer->Renderer, r, g, b, 255);
}

static void 
FigureEntityHighlightFigureArea(figure_entity *FigureEntity, 
                                game_offscreen_buffer *Buffer,
                                SDL_Color Color, u32 Thickness)
{
    game_rect AreaQuad = FigureEntity->FigureArea;
    
    u8 r, g, b;
    SDL_GetRenderDrawColor(Buffer->Renderer, &r, &g, &b, 0);
    SDL_SetRenderDrawColor(Buffer->Renderer, Color.r, Color.g, Color.b, FigureEntity->AreaAlpha);
    
    for(u32 i = 0; i < Thickness; i ++)
    {
        SDL_RenderDrawRect(Buffer->Renderer, &AreaQuad);
        
        AreaQuad.x += 1;
        AreaQuad.y += 1;
        AreaQuad.w -= 2;
        AreaQuad.h -= 2;
    }
    
    SDL_SetRenderDrawColor(Buffer->Renderer, r, g, b, 255);
}

#if 0
static void
DEBUGPrintEntityOrder(figure_entity *FigureEntity)
{
    figure_unit *FigureHead = FigureEntity->HeadFigure;
    while(FigureHead)
    {
        printf("%d ", FigureHead->Index);
        FigureHead = FigureHead->Next;
    }
    printf("\n");
}

#endif

static u32
GameResizeActiveBlock(u32 GridAreaWidth, 
                      u32 GridAreaHeight, 
                      u32 RowAmount, 
                      u32 ColumnAmount)
{
    u32 ResultBlockSize = 0;
    
    u32 DefaultBlockWidth  = GridAreaWidth  / (ColumnAmount + 1);
    u32 DefaultBlockHeight = GridAreaHeight / (RowAmount + 1);
    u32 DefaultBlockSize   = DefaultBlockWidth < DefaultBlockHeight ? DefaultBlockWidth : DefaultBlockHeight;
    
    ResultBlockSize = DefaultBlockSize;
    ResultBlockSize = ResultBlockSize - (ResultBlockSize % 2);
    
    return(ResultBlockSize);
}

static u32
GameResizeInActiveBlock(u32 FigureAreaWidth, 
                        u32 FigureAreaHeight, 
                        u32 DefaultBlocksInRow,
                        u32 DefaultBlocksInCol,
                        u32 BlocksInRow)
{
    u32 ResultBlockSize  = 0;
    
    u32 DefaultBlockWidth  = FigureAreaWidth / DefaultBlocksInRow;
    u32 DefaultBlockHeight = FigureAreaHeight / DefaultBlocksInCol;
    u32 DefaultBlockSize   = DefaultBlockWidth < DefaultBlockHeight ? DefaultBlockWidth : DefaultBlockHeight;
    u32 ActualBlockSize    = FigureAreaWidth / BlocksInRow;
    
    ResultBlockSize = ActualBlockSize < DefaultBlockSize ? ActualBlockSize : DefaultBlockSize;
    ResultBlockSize = ResultBlockSize - (ResultBlockSize % 2);
    
    return(ResultBlockSize);
}

static void
GameRenderBitmapToBuffer(game_offscreen_buffer *Buffer, game_texture *&Texture, game_rect *Quad)
{
    SDL_RenderCopy(Buffer->Renderer, Texture, 0, Quad);
}

#if 0
static void
GameCopyImageToBuffer(game_bitmap* GameBitmap, u32 X, u32 Y,
                      game_offscreen_buffer *Buffer)
{
    u8 BytesPerPixel = GameBitmap->BytesPerPixel;
    
    u8 *RowBuffer = (u8*)Buffer->Memory + (Y * GameBitmap->Pitch);
    u8 *RowTarget = (u8*)GameBitmap->Pixels;
    
    for (u32 y = 0; y < GameBitmap->Height; ++y)
    {
        u32 *PixelBuffer = (u32*)RowBuffer + X;
        u32 *PixelTarget = (u32*)RowTarget;
        
        for (u32 x = 0; x < GameBitmap->Width; ++x)
        {
            u8 Alpha = (*PixelTarget >> 24) & 0xFF;
            u8 Blue  = (*PixelTarget >> 16) & 0xFF;
            u8 Green = (*PixelTarget >> 8)  & 0xFF;
            u8 Red   = (*PixelTarget & 0xFF);
            
            *PixelBuffer = ((Red << 24) | (Green << 16) | (Blue << 8) | (Alpha));
            
            *PixelBuffer++;
            *PixelTarget++;
        }
        
        RowBuffer += Buffer->Pitch;
        RowTarget += GameBitmap->Width * BytesPerPixel;
    }
    
}
#endif

#if 0
static figure_unit*
GetFigureUnitAt(figure_entity *Group, u32 Index)
{
    u32 Size = Group->FigureAmount;
    
    figure_unit *Figure = Group->HeadFigure;
    for (u32 i = 0; i < Group->FigureAmount; ++i)
    {
        if(Figure->Index == Index)
        {
            return Figure;
        }
        
        Figure = Figure->Next;
    }
}

#endif

inline static bool
IsPointInsideRect(s32 X, s32 Y, game_rect *Quad)
{
    if(!Quad) return false;
    
    if(X <= Quad->x)                 return false;
    else if(Y <= Quad->y)            return false;
    else if(X > (Quad->x + Quad->w)) return false;
    else if(Y > (Quad->y + Quad->h)) return false;
    else                             return true;
}

inline static bool
IsFigureUnitInsideRect(figure_unit *Unit, game_rect *AreaQuad)
{
    for (u32 i = 0; i < 4; ++i)
    {
        if(IsPointInsideRect(Unit->Shell[i].x, Unit->Shell[i].y, AreaQuad))
        {
            return true;
        }
    }
    
    return false;
}

static void
FigureEntityHighOrderFigure(figure_entity *FigureEntity, u32 Index)
{
    u32 StartIndex   = 0;
    u32 ActiveIndex  = FigureEntity->FigureActive;
    u32 FigureAmount = FigureEntity->FigureAmount;
    u32 *FigureOrder = FigureEntity->FigureOrder;
    
    if(ActiveIndex != FigureOrder[FigureAmount - 1])
    {
        for (u32 i = 0; i < FigureAmount; i++)
        {
            if(FigureOrder[i] == Index)
            {
                StartIndex= i;
            }
        }
        
        for (u32 i = StartIndex; i < FigureAmount - 1; ++i)
        {
            FigureOrder[i] = FigureOrder[i+1];
        }
        
        FigureOrder[FigureAmount-1] = Index;
    }
    
}

static void 
FigureEntityLowPriority(figure_entity *FigureEntity, u32 Index)
{
    u32 StartIndex   = 0;
    u32 FigureAmount = FigureEntity->FigureAmount;
    u32 *FigureOrder = FigureEntity->FigureOrder;
    
    if(FigureOrder[0] == Index)
    {
        return;
    }
    
    for (u32 i = 0; i < FigureAmount; i++)
    {
        if(FigureOrder[i] == Index)
        {
            StartIndex = i;
            break;
        }
    }
    
    
    for (u32  i = StartIndex; i > 0; i--)
    {
        FigureOrder[i] = FigureOrder[i-1];
    }
    
    FigureOrder[0] = Index;
}

#if 0
static void
FigureUnitSwapAtEnd(figure_unit *&Head, u32 FigureIndex)
{
    if(Head->Next == NULL) return;
    
    figure_unit *TargetNode  = NULL;
    figure_unit *PrevNode    = NULL;
    figure_unit *CurrentNode = Head;
    
    while(CurrentNode)
    {
        if(CurrentNode->Index == FigureIndex)
        {
            if(PrevNode)
            {
                if(CurrentNode->Next)
                {
                    PrevNode->Next = CurrentNode->Next;
                }
                else
                {
                    PrevNode->Next = CurrentNode;
                }
            }
            else
            {
                Head = CurrentNode->Next;
            }
            
            TargetNode = CurrentNode;
        }
        
        PrevNode    = CurrentNode;
        CurrentNode = CurrentNode->Next;
    }
    
    PrevNode->Next   = TargetNode;
    TargetNode->Next = NULL;
}



static void
FigureUnitSwapAtBeginning(figure_unit *&FigureHead, u32 Index)
{
    if(FigureHead->Index == Index) return;
    
    figure_unit *TargetNode = NULL;
    figure_unit *PrevNode   = NULL;
    figure_unit *CurrentNode = FigureHead;
    
    while(CurrentNode && 
          Index != CurrentNode->Index)
    {
        PrevNode = CurrentNode;
        CurrentNode = CurrentNode->Next;
    }
    
    PrevNode->Next = CurrentNode->Next;
    CurrentNode->Next = FigureHead;
    FigureHead = CurrentNode;
}

#endif

static void
FigureUnitResizeBy(figure_unit *Entity, r32 ScaleFactor)
{
    game_rect *Rectangle = &Entity->AreaQuad;
    game_point OldCenter = {};
    game_point NewCenter = {};
    
    s32 OffsetX = 0;
    s32 OffsetY = 0;
    s32 OldX = Entity->AreaQuad.x;
    s32 OldY = Entity->AreaQuad.y;
    
    OldCenter.x = Entity->AreaQuad.x + (Entity->AreaQuad.w / 2);
    OldCenter.y = Entity->AreaQuad.y + (Entity->AreaQuad.h / 2);
    
    Rectangle->w = roundf(Rectangle->w * ScaleFactor);
    Rectangle->h = roundf(Rectangle->h * ScaleFactor);
    
    NewCenter.x = Entity->AreaQuad.x + (Entity->AreaQuad.w / 2);
    NewCenter.y = Entity->AreaQuad.y + (Entity->AreaQuad.h / 2);
    
    Rectangle->x += (OldCenter.x - NewCenter.x);
    Rectangle->y += (OldCenter.y - NewCenter.y);
    
    OffsetX = roundf((Entity->Center.x - OldX) * ScaleFactor);
    OffsetY = roundf((Entity->Center.y - OldY) * ScaleFactor);
    Entity->Center.x = OldX + OffsetX;
    Entity->Center.y = OldY + OffsetY;
    Entity->Center.x += (OldCenter.x - NewCenter.x);
    Entity->Center.y += (OldCenter.y - NewCenter.y);
    
    for (u32 i = 0; i < 4; ++i)
    {
        OffsetX = roundf((Entity->Shell[i].x - OldX) * ScaleFactor);
        OffsetY = roundf((Entity->Shell[i].y - OldY) * ScaleFactor);
        Entity->Shell[i].x = OldX + OffsetX;
        Entity->Shell[i].y = OldY + OffsetY;
        Entity->Shell[i].x += (OldCenter.x - NewCenter.x);
        Entity->Shell[i].y += (OldCenter.y - NewCenter.y);
    }
    
}

static void
FigureUnitAddNewFigure(figure_entity *FigureEntity, char* AssetName, 
              figure_form Form, figure_type Type, 
              game_memory *Memory, game_offscreen_buffer *Buffer)
{
    if(FigureEntity->FigureAmount >= FigureEntity->FigureAmountReserved) return;
    
    u32 ActiveBlockSize   = Memory->LevelEntity.ActiveBlockSize;
    u32 InActiveBlockSize = Memory->LevelEntity.InActiveBlockSize;
    
    u32 Index = FigureEntity->FigureAmount;
    
    FigureEntity->FigureUnit[Index].IsIdle       = true;
    FigureEntity->FigureUnit[Index].IsStick      = false;
    FigureEntity->FigureUnit[Index].IsEnlarged   = false;
    FigureEntity->FigureUnit[Index].Angle        = 0.0f;
    FigureEntity->FigureUnit[Index].DefaultAngle = 0.0f;
    FigureEntity->FigureUnit[Index].Form         = Form;
    FigureEntity->FigureUnit[Index].Type         = Type;
    FigureEntity->FigureUnit[Index].Flip         = SDL_FLIP_NONE;
    FigureEntity->FigureUnit[Index].Texture      = GetTexture(Memory, AssetName, Buffer->Renderer);
    
    
    u32 RowAmount         = 0;
    u32 ColumnAmount      = 0;
    r32 CenterOffset      = 0.5f;
    vector<vector<s32>> matrix(2);
    for(u32 i = 0; i < 2; ++i)
    {
        matrix[i].resize(4);
    }
    
    switch(Form)
    {
        case I_figure:
        {
            matrix = 
            { 
                {1, 1, 1, 1},
                {0, 0, 0, 0} 
            };
            RowAmount    = 4;
            ColumnAmount = 1;
        } break;
        
        case O_figure:
        {
            matrix = 
            { 
                { 1, 1 },
                { 1, 1 } 
            };
            RowAmount    = 2;
            ColumnAmount = 2;
        }break;
        
        case Z_figure:
        {
            matrix =
            {
                {1, 1, 0}, 
                {0, 1, 1} 
            };
            RowAmount    = 3;
            ColumnAmount = 2;
        }break;
        
        case S_figure:
        {
            matrix =
            { 
                {0, 1, 1}, 
                {1, 1, 0} 
            };
            RowAmount    = 3;
            ColumnAmount = 2;
        }break;
        
        case T_figure:
        {
            matrix = 
            { 
                {0, 1, 0},
                {1, 1, 1}
            };
            CenterOffset = 0.75f;
            RowAmount    = 3;
            ColumnAmount = 2;
        }break;
        
        case L_figure:
        {
            matrix = 
            { 
                {0, 0, 1},
                {1, 1, 1}
            };
            CenterOffset = 0.75f;
            RowAmount    = 3;
            ColumnAmount = 2;
        }break;
        
        case J_figure:
        {
            matrix = 
            {
                {1, 0, 0},
                {1, 1, 1} 
            };
            CenterOffset = 0.75f;
            RowAmount    = 3;
            ColumnAmount = 2;
        }break;
    }
    
    
    FigureEntity->FigureUnit[Index].AreaQuad.x = 0;
    FigureEntity->FigureUnit[Index].AreaQuad.y = 0;
    FigureEntity->FigureUnit[Index].AreaQuad.w = RowAmount * InActiveBlockSize;
    FigureEntity->FigureUnit[Index].AreaQuad.h = ColumnAmount * InActiveBlockSize;
    FigureEntity->FigureUnit[Index].Center.x   = FigureEntity->FigureUnit[Index].AreaQuad.x + (FigureEntity->FigureUnit[Index].AreaQuad.w / 2);
    FigureEntity->FigureUnit[Index].Center.y   = FigureEntity->FigureUnit[Index].AreaQuad.y + (FigureEntity->FigureUnit[Index].AreaQuad.h) * CenterOffset;
    FigureEntity->FigureUnit[Index].DefaultCenter = FigureEntity->FigureUnit[Index].Center;
    
    u32 ShellIndex = 0;
    u32 HalfBlock  = InActiveBlockSize >> 1;
    
    for(u32 i = 0; i < 2; i++)
    {
        for(u32 j = 0; j < 4; j++)
        {
            if(matrix[i][j] == 1)
            {
                FigureEntity->FigureUnit[Index].Shell[ShellIndex].x = FigureEntity->FigureUnit[Index].AreaQuad.x + (j * InActiveBlockSize) + HalfBlock;
                
                FigureEntity->FigureUnit[Index].Shell[ShellIndex].y = FigureEntity->FigureUnit[Index].AreaQuad.y + (i * InActiveBlockSize) + HalfBlock;
                
                FigureEntity->FigureUnit[Index].DefaultShell[ShellIndex] = FigureEntity->FigureUnit[Index].Shell[ShellIndex];
                
                ShellIndex++;
            }
        }
    }
    
    FigureEntity->FigureAmount += 1;
}

static void
CreateFigureUnit(figure_unit* Figure, char* AssetName, 
                 figure_form Form,figure_type Type,
                 game_memory *Memory, game_offscreen_buffer *Buffer)
{
    if (!Figure) return;
    
    //Figure->Index        = EntityIndex;
    Figure->IsIdle       = true;
    Figure->IsStick      = false;
    Figure->IsEnlarged   = false;
    Figure->Angle        = 0.0f;
    Figure->DefaultAngle = 0.0f;
    Figure->Form = Form;
    Figure->Type = Type;
    Figure->Flip = SDL_FLIP_NONE;
    Figure->Texture = GetTexture(Memory, AssetName, Buffer->Renderer);
    SDL_SetTextureBlendMode(Figure->Texture, SDL_BLENDMODE_BLEND);
    
    int RowAmount       = 0;
    int ColumnAmount    = 0;
    u32 BlockSize       = Memory->LevelEntity.InActiveBlockSize;
    float CenterOffset  = 0.5f;
    vector<vector<int>> matrix(2);
    for (int i = 0; i < 2; i++)
    {
        matrix[i].resize(4);
    }
    
    switch(Form)
    {
        case I_figure:
        {
            matrix = 
            { 
                {1, 1, 1, 1},
                {0, 0, 0, 0} 
            };
            RowAmount    = 4;
            ColumnAmount = 1;
        } break;
        
        case O_figure:
        {
            matrix = 
            { 
                { 1, 1 },
                { 1, 1 } 
            };
            RowAmount    = 2;
            ColumnAmount = 2;
        }break;
        
        case Z_figure:
        {
            matrix =
            {
                {1, 1, 0}, 
                {0, 1, 1} 
            };
            RowAmount    = 3;
            ColumnAmount = 2;
        }break;
        
        case S_figure:
        {
            matrix =
            { 
                {0, 1, 1}, 
                {1, 1, 0} 
            };
            RowAmount    = 3;
            ColumnAmount = 2;
        }break;
        
        case T_figure:
        {
            matrix = 
            { 
                {0, 1, 0},
                {1, 1, 1}
            };
            CenterOffset = 0.75f;
            RowAmount    = 3;
            ColumnAmount = 2;
        }break;
        
        case L_figure:
        {
            matrix = 
            { 
                {0, 0, 1},
                {1, 1, 1}
            };
            CenterOffset = 0.75f;
            RowAmount    = 3;
            ColumnAmount = 2;
        }break;
        
        case J_figure:
        {
            matrix = 
            {
                {1, 0, 0},
                {1, 1, 1} 
            };
            CenterOffset = 0.75f;
            RowAmount    = 3;
            ColumnAmount = 2;
        }break;
    }
    
    Figure->AreaQuad.x = 0;
    Figure->AreaQuad.y = 0;
    Figure->AreaQuad.w = RowAmount*BlockSize;
    Figure->AreaQuad.h = ColumnAmount*BlockSize;
    Figure->Center.x = Figure->AreaQuad.x + (Figure->AreaQuad.w / 2);
    Figure->Center.y = Figure->AreaQuad.y + (Figure->AreaQuad.h) * CenterOffset;
    Figure->DefaultCenter = Figure->Center;
    
    int Index     = 0;
    u32 HalfBlock = BlockSize >> 1;
    
    for (u32 i = 0; i < 2; i++) 
    {
        for (u32 j = 0; j < 4; j++) 
        {
            if(matrix[i][j] == 1) 
            {
                Figure->Shell[Index].x      = Figure->AreaQuad.x + (j * BlockSize) + HalfBlock;
                Figure->Shell[Index].y      = Figure->AreaQuad.y + (i * BlockSize) + HalfBlock;
                Figure->DefaultShell[Index] = Figure->Shell[Index];
                Index++;
            }
        }
    }
    
    
}

static game_rect
FigureUnitGetArea(figure_unit *Unit)
{
    game_rect Area = {Unit->Shell[0].x, Unit->Shell[0].y, -500, -500};
    u32 OffsetX = 0;
    u32 OffsetY = 0;
    bool ZeroArea = false;
    
    for (u32 i = 0; i < 4; ++i)
    {
        if(Area.x >= Unit->Shell[i].x) Area.x = Unit->Shell[i].x;
        if(Area.y >= Unit->Shell[i].y) Area.y = Unit->Shell[i].y;
        if(Area.w <= Unit->Shell[i].x) Area.w = Unit->Shell[i].x;
        if(Area.h <= Unit->Shell[i].y) Area.h = Unit->Shell[i].y;
        
    }
    
    Area.w -= Area.x;
    Area.h -= Area.y;
    
    if(Area.w == 0 || Area.h == 0)
    {
        ZeroArea = true;
    }
    
    if(Area.w >= Unit->AreaQuad.h)
    {
        Area.h = Unit->AreaQuad.h;
        Area.w = Unit->AreaQuad.w;
        OffsetX = (Area.y + Area.h) - (Area.y + Area.h / 2);
    }
    else
    {
        Area.h = Unit->AreaQuad.w;
        Area.w = Unit->AreaQuad.h;
        OffsetX = (Area.x + Area.w) - (Area.x + Area.w / 2);
    }
    
    if(!ZeroArea)
    {
        OffsetX /= 2;
    }
    
    Area.x -= OffsetX;
    Area.y -= OffsetX;
    
    return(Area);
}

static void
FigureUnitRotateShellBy(figure_unit *Entity, float Angle)
{
    if((s32)Entity->Angle == 0)
    {
        Entity->Angle = 0;
    }
    
    for (u32 i = 0; i < 4; ++i)
    {
        float Radians = Angle * (M_PI/180.0f);
        float Cos = cos(Radians);
        float Sin = sin(Radians);
        
        float X = Entity->Center.x + (Entity->Shell[i].x - Entity->Center.x) * Cos
            - (Entity->Shell[i].y - Entity->Center.y) * Sin;
        float Y = Entity->Center.y + (Entity->Shell[i].x - Entity->Center.x) * Sin
            + (Entity->Shell[i].y - Entity->Center.y) * Cos;
        
        Entity->Shell[i].x = roundf(X);
        Entity->Shell[i].y = roundf(Y);
    }
}

static void
FigureUnitFlipHorizontally(figure_unit *Unit)
{
    u32 NewX = 0;
    u32 NewY = 0;
    u32 NewCenterX = 0;
    u32 NewCenterY = 0;
    game_rect AreaQuad = FigureUnitGetArea(Unit);
    
    if(Unit->Flip != SDL_FLIP_HORIZONTAL)
    {
        Unit->Flip = SDL_FLIP_HORIZONTAL;
    }
    else
    {
        Unit->Flip = SDL_FLIP_NONE;
    }
    
    if(AreaQuad.w > AreaQuad.h)
    {
        for (u32 i = 0; i < 4; ++i)
        {
            NewX = (AreaQuad.x + AreaQuad.w) - (Unit->Shell[i].x - AreaQuad.x);
            Unit->Shell[i].x = NewX;
        }
        
        NewCenterX = (AreaQuad.x + AreaQuad.w) - (Unit->Center.x - AreaQuad.x);
        Unit->Center.x = NewCenterX;
    }
    else
    {
        for (u32 i = 0; i < 4; ++i)
        {
            NewY = (AreaQuad.y + AreaQuad.h) - (Unit->Shell[i].y - AreaQuad.y);
            Unit->Shell[i].y = NewY;
        }
        
        NewCenterY = (AreaQuad.y + AreaQuad.h) - (Unit->Center.y - AreaQuad.y);
        Unit->Center.y = NewCenterY;
    }
}


static void
FigureUnitMove(figure_unit *Entity, s32 XShift, s32 YShift)
{
    int XOffset = Entity->AreaQuad.x - Entity->Center.x;
    int YOffset = Entity->AreaQuad.y - Entity->Center.y;
    
    Entity->Center.x += XShift;
    Entity->Center.y += YShift;
    
    Entity->AreaQuad.x = Entity->Center.x + XOffset;
    Entity->AreaQuad.y = Entity->Center.y + YOffset;
    
    for (u32 i = 0; i < 4; ++i)
    {
        Entity->Shell[i].x += XShift;
        Entity->Shell[i].y += YShift;
    }
}

static void
FigureUnitMoveTo(figure_unit *Entity, s32 NewPointX, s32 NewPointY)
{
    s32 XShift = NewPointX - Entity->Center.x;
    s32 YShift = NewPointY - Entity->Center.y;
    FigureUnitMove(Entity, XShift, YShift);
}

static void
DestroyFigureEntity(figure_unit *Entity)
{
    if(Entity)
    {
        FreeTexture(Entity->Texture);
        free(Entity);
    }
}

static void
FigureUnitRenderBitmap(game_offscreen_buffer *Buffer, figure_unit *Entity)
{
    game_point Center;
    Center.x = Entity->Center.x - Entity->AreaQuad.x;
    Center.y = Entity->Center.y - Entity->AreaQuad.y;
    
    SDL_RenderCopyEx(Buffer->Renderer, Entity->Texture,
                     0, &Entity->AreaQuad, Entity->Angle, &Center, Entity->Flip);
}


static void
FigureUnitSetToDefaultArea(figure_unit* Unit, r32 BlockRatio)
{
    s32 ShiftX = 0;
    s32 ShiftY = 0;
    
    if(Unit->IsEnlarged)
    {
        r32 AngleDt      = 0;
        r32 Angle        = Unit->Angle;
        r32 DefaultAngle = Unit->DefaultAngle;
        
        if(Unit->Angle != Unit->DefaultAngle)
        {
            AngleDt = DefaultAngle - Angle;
            FigureUnitRotateShellBy(Unit, AngleDt);
            Unit->Angle = DefaultAngle;
        }
        
        FigureUnitResizeBy(Unit, BlockRatio);
        
        Unit->IsEnlarged = false;
    }
}

static void
FigureUnitDefineDefaultArea(figure_unit *Unit, s32 X, s32 Y)
{
    game_rect AreaQuad = FigureUnitGetArea(Unit);
    s32 ShiftX = X - AreaQuad.x;
    s32 ShiftY = Y - AreaQuad.y;
    
    FigureUnitMove(Unit, ShiftX, ShiftY);
    
    for (u32 i = 0; i < 4; ++i)
    {
        Unit->DefaultShell[i] = Unit->Shell[i];
    }
    
    Unit->DefaultCenter = Unit->Center;
    Unit->DefaultAngle  = Unit->Angle;
}

static void
FigureUnitMoveToDefaultArea(figure_unit *FigureUnit, u32 ActiveBlockSize)
{
    game_point Center        = FigureUnit->Center;
    game_point DefaultCenter = FigureUnit->DefaultCenter;
    
    r32 OffsetX   = 0.0f;
    r32 OffsetY   = 0.0f;
    r32 Magnitude = 0.0f;
    r32 MaxSpeed  = 0.0f;
    
    OffsetX = DefaultCenter.x - Center.x;
    OffsetY = DefaultCenter.y - Center.y;
    
    Magnitude = sqrt(OffsetX*OffsetX + OffsetY*OffsetY);
    MaxSpeed = Magnitude - ActiveBlockSize;
    if(Magnitude > (MaxSpeed))
    {
        if(Magnitude != 0) 
        {
            OffsetX /= Magnitude;
            OffsetY /= Magnitude;
        }
        
        OffsetX *= MaxSpeed;
        OffsetY *= MaxSpeed;
        
        OffsetX = roundf(OffsetX);
        OffsetY = roundf(OffsetY);
        FigureUnitMove(FigureUnit, OffsetX, OffsetY);
    }
}

static void
GridEntityAddMovingBlock(grid_entity *GridEntity,
                          u32 RowNumber, u32 ColNumber, 
                          bool IsVertical, bool MoveSwitch, 
                          u32 ActiveBlockSize)
{
    if(GridEntity->MovingBlocksAmount >= GridEntity->MovingBlocksAmountReserved) return;
    if((RowNumber >= GridEntity->RowAmount) || RowNumber < 0) return;
    if((ColNumber >= GridEntity->ColumnAmount) || ColNumber < 0) return;
    
    u32 Index = GridEntity->MovingBlocksAmount;
    
    GridEntity->MovingBlocks[Index].RowNumber = RowNumber;
    GridEntity->MovingBlocks[Index].RowNumber = ColNumber;
    
    GridEntity->MovingBlocks[Index].AreaQuad.w = ActiveBlockSize;
    GridEntity->MovingBlocks[Index].AreaQuad.h = ActiveBlockSize;
    GridEntity->MovingBlocks[Index].AreaQuad.x = GridEntity->GridArea.x + (ColNumber * ActiveBlockSize);
    GridEntity->MovingBlocks[Index].AreaQuad.y = GridEntity->GridArea.y + (RowNumber * ActiveBlockSize);
    
    GridEntity->MovingBlocks[Index].IsMoving   = false;
    GridEntity->MovingBlocks[Index].MoveSwitch = MoveSwitch;
    GridEntity->MovingBlocks[Index].IsVertical = IsVertical;
    GridEntity->MovingBlocks[Index].RowNumber  = RowNumber;
    GridEntity->MovingBlocks[Index].ColNumber  = ColNumber;
    
    GridEntity->UnitField[RowNumber][ColNumber] = IsVertical ? 3 : 2;
    
    GridEntity->MovingBlocksAmount += 1;
}

static void
GridEntityMoveBlockHorizontally(grid_entity *GridEntity, moving_block *MovingBlock)
{
    s32 NewColNumber = 0;
    u32 RowNumber = MovingBlock->RowNumber;
    u32 ColNumber = MovingBlock->ColNumber;
    u32 ActiveBlockSize = MovingBlock->AreaQuad.w;
    
    NewColNumber = MovingBlock->MoveSwitch
        ? NewColNumber = ColNumber + 1
        : NewColNumber = ColNumber - 1;
    
    if(NewColNumber < 0 || NewColNumber >= GridEntity->ColumnAmount) return;
    if(GridEntity->UnitField[RowNumber][NewColNumber] != 0) return;
    
    GridEntity->UnitField[RowNumber][ColNumber]    = 0;
    GridEntity->UnitField[RowNumber][NewColNumber] = 2;
    
    MovingBlock->ColNumber = NewColNumber;
    
    MovingBlock->MoveSwitch = MovingBlock->MoveSwitch == false ? true : false;
    MovingBlock->IsMoving = true;
}

static void
GridEntityMoveBlockVertically(grid_entity *GridEntity, moving_block *MovingBlock)
{
    s32 NewRowNumber = 0;
    u32 RowNumber = MovingBlock->RowNumber;
    u32 ColNumber = MovingBlock->ColNumber;
    u32 ActiveBlockSize = MovingBlock->AreaQuad.w;
    
    NewRowNumber = MovingBlock->MoveSwitch
        ? NewRowNumber = RowNumber + 1
        : NewRowNumber = RowNumber - 1;
    
    if(NewRowNumber < 0 || NewRowNumber >= GridEntity->RowAmount) return;
    if(GridEntity->UnitField[NewRowNumber][ColNumber] != 0) return;
    
    GridEntity->UnitField[RowNumber][ColNumber]    = 0;
    GridEntity->UnitField[NewRowNumber][ColNumber] = 3;
    
    MovingBlock->RowNumber = NewRowNumber;
    
    MovingBlock->MoveSwitch = MovingBlock->MoveSwitch == false ? true : false;
    MovingBlock->IsMoving = true;
}

static void
RestartLevelEntity(level_entity *LevelEntity)
{
    grid_entity   *&GridEntity   = LevelEntity->GridEntity;
    figure_entity *&FigureEntity = LevelEntity->FigureEntity;
    
    
    if(FigureEntity->IsRotating || FigureEntity->IsFlipping) return;
    
    r32 BlockRatio        = 0;
    u32 RowAmount         = 0;
    u32 ColumnAmount      = 0;
    u32 FigureAmount      = FigureEntity->FigureAmount;
    r32 ActiveBlockSize   = LevelEntity->ActiveBlockSize;
    r32 InActiveBlockSize = LevelEntity->InActiveBlockSize;
    
    FigureEntity->IsRestarting = true;
    
    for(u32 i = 0; i < FigureAmount; ++i){
        if(!FigureEntity->FigureUnit[i].IsIdle){
            
            FigureEntity->FigureUnit[i].IsStick = false;
            BlockRatio = InActiveBlockSize / ActiveBlockSize;
            FigureUnitSetToDefaultArea(&FigureEntity->FigureUnit[i], BlockRatio);
            FigureUnitMoveToDefaultArea(&FigureEntity->FigureUnit[i], ActiveBlockSize);
            
            printf("BlockRatio = %f\n" ,BlockRatio);
        }
    }
    
    for(u32 i = 0; i < GridEntity->StickUnitsAmount; ++i)
    {
        s32 Index = GridEntity->StickUnits[i].Index;
        
        if(GridEntity->StickUnits[i].IsSticked && Index >= 0)
        {
            s32 RowIndex = 0;
            s32 ColIndex = 0;
            
            for(u32 l = 0; l < 4; l++)
            {
                RowIndex = GridEntity->StickUnits[i].Row[l];
                ColIndex = GridEntity->StickUnits[i].Col[l];
                GridEntity->UnitField[RowIndex][ColIndex] = 0;
            }
            
            GridEntity->StickUnits[i].Index = -1;
        }
    }
}

static void
GameUpdateEvent(game_input *Input, level_entity *GameState,
                r32 ActiveBlockSize, r32 DefaultBlockSize,
                u32 ScreenWidth, u32 ScreenHeight)
{
    if(!GameState->LevelStarted) return;
    if(GameState->LevelPaused)   return;
    
    grid_entity   *&GridEntity   = GameState->GridEntity;
    figure_entity *&FigureEntity = GameState->FigureEntity;
    figure_unit   *FigureUnit    = FigureEntity->FigureUnit;
    
    u32 Size        = FigureEntity->FigureAmount;
    s32 MouseX      = Input->MouseX;
    s32 MouseY      = Input->MouseY;
    r32 BlockRatio  = 0;
    u32 ActiveIndex = FigureEntity->FigureActive;
    
    if(Input->WasPressed)
    {
        
        
        if(Input->LeftClick.IsDown)
        {
            if(!FigureEntity->IsGrabbed)
            {
                for (s32 i = Size-1; i >= 0; --i)
                {
                    ActiveIndex = FigureEntity->FigureOrder[i];
                    game_rect AreaQuad = FigureUnitGetArea(&FigureUnit[ActiveIndex]);
                    if(IsPointInsideRect(MouseX, MouseY, &AreaQuad))
                    {
                        game_rect ShellQuad = {0};
                        ShellQuad.w = FigureUnit[ActiveIndex].IsEnlarged ? ActiveBlockSize : DefaultBlockSize;
                        ShellQuad.h = FigureUnit[ActiveIndex].IsEnlarged ? ActiveBlockSize : DefaultBlockSize;
                        for(u32 j = 0; j < 4; ++j)
                        {
                            ShellQuad.x = FigureUnit[ActiveIndex].Shell[j].x - (ShellQuad.w / 2);
                            ShellQuad.y = FigureUnit[ActiveIndex].Shell[j].y - (ShellQuad.h / 2);
                            if(IsPointInsideRect(MouseX, MouseY, &ShellQuad))
                            {
                                FigureEntity->IsGrabbed = true;
                                
                                if(!FigureUnit[ActiveIndex].IsEnlarged)
                                {
                                    BlockRatio = ActiveBlockSize / DefaultBlockSize;
                                    FigureUnit[ActiveIndex].IsIdle = false;
                                    FigureUnit[ActiveIndex].IsEnlarged = true;
                                    FigureUnitResizeBy(&FigureUnit[ActiveIndex], BlockRatio);
                                }
                                
                                FigureEntity->FigureActive = ActiveIndex;
                                FigureEntityHighOrderFigure(FigureEntity, ActiveIndex);
                                SDL_ShowCursor(SDL_DISABLE);
                                //DEBUGPrintEntityOrder(Group);
                                
                                return;
                            }
                        }
                    }
                }
                
                if(IsPointInsideRect(MouseX, MouseY, &GridEntity->GridArea))
                {
                    for(u32 i = 0; i < GridEntity->MovingBlocksAmount; i++)
                    {
                        if(IsPointInsideRect(MouseX, MouseY, &GridEntity->MovingBlocks[i].AreaQuad))
                        {
                            if(!GridEntity->MovingBlocks[i].IsVertical) 
                            {
                                GridEntityMoveBlockHorizontally(GridEntity, &GridEntity->MovingBlocks[i]);
                                printf("index of moving block = %d\n", i);
                            }
                            else
                            {
                                GridEntityMoveBlockVertically(GridEntity, &GridEntity->MovingBlocks[i]);
                            }
                            
                            return;
                        }
                    }
                }
            }
            else
            {
                if(!FigureEntity->IsRotating && !FigureEntity->IsFlipping)
                {
                    game_rect ScreenArea = {0};
                    ScreenArea.w = ScreenWidth;
                    ScreenArea.h = ScreenHeight;
                    
                    if(IsFigureUnitInsideRect(&FigureUnit[ActiveIndex], &FigureEntity->FigureArea) || 
                       (!IsPointInsideRect(FigureUnit[ActiveIndex].Center.x,FigureUnit[ActiveIndex].Center.y, &ScreenArea)))
                    {
                        FigureUnit[ActiveIndex].IsIdle = true;
                        BlockRatio = DefaultBlockSize / ActiveBlockSize;
                        FigureUnitSetToDefaultArea(&FigureUnit[ActiveIndex], BlockRatio);
                        
                        FigureEntity->IsReturning = true;
                        FigureEntity->ReturnIndex = ActiveIndex;
                        
                        FigureUnitMoveToDefaultArea(&FigureUnit[ActiveIndex], ActiveBlockSize);
                    }
                    
                    SDL_ShowCursor(SDL_ENABLE);
                    FigureEntity->IsGrabbed = false;
                    FigureEntity->FigureActive = -1;
                }
            }
        }
        if(Input->RightClick.IsDown)
        {
            if(FigureEntity->IsGrabbed)
            {
                figure_type Type = FigureUnit[ActiveIndex].Type;
                switch(Type)
                {
                    case classic:
                    {
                        if (!FigureEntity->IsRotating)
                        {
                            FigureEntity->IsRotating = true;
                            FigureUnitRotateShellBy(&FigureUnit[ActiveIndex], 90);
                        }
                    } break;
                    case mirror:
                    {
                        if (!FigureEntity->IsFlipping)
                        {
                            
                            SDL_SetTextureBlendMode(FigureUnit[ActiveIndex].Texture, SDL_BLENDMODE_BLEND);
                            
                            FigureEntity->IsFlipping = true;
                            FigureEntity->FigureAlpha= 255;
                            FigureEntity->FadeInSum  = 255;
                            FigureEntity->FadeOutSum = 0;
                        }
                    } break;
                    
                    case stone:
                    {
                        
                    } break;
                }
            }
        } 
    }
    if(Input->MouseMotion)
    {
        if(FigureEntity->IsGrabbed)
        {
            s32 x = Input->MouseRelX;
            s32 y = Input->MouseRelY;
            FigureUnitMove(&FigureUnit[ActiveIndex], x, y);
            
        }           
    }
}


static void
FigureEntityAlignHorizontally(figure_entity* Entity, u32 BlockSize)
{
    u32 Size = Entity->FigureAmount;
    u32 RowSize1 = 0;
    u32 RowSize2 = 0;
    u32 FigureIntervalX = BlockSize / 4;
    u32 FigureIntervalY = BlockSize / 6;
    u32 FigureWidth     = 0;
    u32 FigureHeight    = 0;
    
    game_rect AreaQuad    = {};
    game_rect DefaultZone = Entity->FigureArea;
    
    for (u32 i = 0; i < Size; ++i)
    {
        FigureWidth  = Entity->FigureUnit[i].AreaQuad.w;
        FigureHeight = Entity->FigureUnit[i].AreaQuad.h;
        
        if(FigureWidth > FigureHeight)
        {
            FigureUnitRotateShellBy(&Entity->FigureUnit[i], 90.0);
            Entity->FigureUnit[i].Angle += 90.0;
        }
        
        AreaQuad = FigureUnitGetArea(&Entity->FigureUnit[i]);
        i % 2 == 0
            ? RowSize1 += AreaQuad.w + FigureIntervalX
            : RowSize2 += AreaQuad.w + FigureIntervalX;
    }
    
    u32 PitchY          = 0;
    s32 NewPositionX    = 0;
    s32 NewPositionY    = 0;
    u32 CurrentRowSize1 = 0;
    u32 CurrentRowSize2 = 0;
    u32 FigureBoxHeight = 0;
    
    for (u32 i = 0; i < Size; ++i)
    {
        PitchY = i % 2;
        AreaQuad = FigureUnitGetArea(&Entity->FigureUnit[i]);
        
        FigureBoxHeight = BlockSize * 4;
        NewPositionY = DefaultZone.y + (FigureBoxHeight * PitchY);
        NewPositionY += (FigureBoxHeight / 2 ) - (AreaQuad.h / 2);
        NewPositionY += FigureIntervalY * PitchY;
        
        if(i % 2 == 0)
        {
            NewPositionX = (DefaultZone.x + (DefaultZone.w / 2) - RowSize1 / 2);
            NewPositionX += CurrentRowSize1;
            
            CurrentRowSize1 += AreaQuad.w + FigureIntervalX;
        }
        else
        {
            NewPositionX = (DefaultZone.x + (DefaultZone.w / 2)) - (RowSize2 / 2);
            NewPositionX += CurrentRowSize2;
            
            CurrentRowSize2 += AreaQuad.w + FigureIntervalX;
        }
        
        FigureUnitDefineDefaultArea(&Entity->FigureUnit[i], NewPositionX, NewPositionY);
    }
}

inline static bool
Change1DUnitPerSec(r32 *Unit, r32 MaxValue, r32 ChangePerSec, r32 TimeElapsed)
{
    bool IsFinished = true;
    r32 UnitValue = *Unit;
    
    if(MaxValue > 0)
    {
        
        if(UnitValue < MaxValue)
        {
            UnitValue += TimeElapsed * ChangePerSec;
            UnitValue = roundf(UnitValue);
            
            if(UnitValue >= MaxValue) UnitValue = MaxValue;
            
            *Unit = roundf(UnitValue);
            IsFinished = false;
        }
    }
    else
    {
        
        if(UnitValue > MaxValue)
        {
            UnitValue -= TimeElapsed * ChangePerSec;
            UnitValue = roundf(UnitValue);
            
            if(UnitValue <= MaxValue) UnitValue = MaxValue;
            
            *Unit = roundf(UnitValue);
            IsFinished = false;
        }
    }
    
    return(IsFinished);
}

inline static vector2
Move2DPointPerSec(game_point *p1, game_point *p2, r32 MaxVelocity, r32 TimeElapsed)
{
    r32 Distance;
    vector2 Velocity;
    
    Velocity.x= p2->x - p1->x;
    Velocity.y= p2->y - p1->y;
    
    Distance = sqrt(Velocity.x * Velocity.x + Velocity.y * Velocity.y);
    if(Distance > MaxVelocity)
    {
        Velocity.x = Velocity.x / Distance;
        Velocity.y = Velocity.y / Distance;
        
        Velocity.x = Velocity.x * MaxVelocity;
        Velocity.y = Velocity.y * MaxVelocity;
    }
    
    Velocity.x = roundf(Velocity.x);
    Velocity.y = roundf(Velocity.y);
    
    return(Velocity);
}

static void
LevelEntityUpdateAndRender(game_offscreen_buffer *Buffer, level_entity *State, r32 TimeElapsed)
{
    static r32 TotalElapsed = SDL_GetTicks();
    
    grid_entity   *&GridEntity   = State->GridEntity;
    figure_entity *&FigureEntity = State->FigureEntity;
    figure_unit   *FigureUnit    = FigureEntity->FigureUnit;
    
    u32 ActiveBlockSize   = State->ActiveBlockSize;
    u32 InActiveBlockSize = State->InActiveBlockSize;
    
    game_rect AreaQuad = {};
    r32 MaxVel       = ActiveBlockSize / 6;
    u32 RowAmount    = GridEntity->RowAmount;
    u32 ColumnAmount = GridEntity->ColumnAmount;
    u32 FigureAmount = FigureEntity->FigureAmount;
    s32 ActiveIndex  = FigureEntity->FigureActive;
    
    bool ToggleHighlight = false;
    
    s32 StartX = 0;
    s32 StartY = 0;
    
    if(!State->LevelStarted)
    {
        bool IsLevelReady = true;
        r32 ScaleSpeed    = TimeElapsed * (State->GridScalePerSec);
        
        for(u32 line = 1; line <=(RowAmount + ColumnAmount - 1); ++line)
        {
            bool ShouldBreak = true;
            s32 StartColumn  = Max2(0, line - RowAmount);
            s32 Count        = Min3(line, (ColumnAmount - StartColumn), RowAmount);
            
            for(u32 i = 0; i < Count; ++i)
            {
                u32 RowIndex  = Min2(RowAmount, line) - i - 1;
                u32 ColIndex  = StartColumn + i;
                r32 *UnitSize  = &GridEntity->UnitSize[RowIndex][ColIndex];
                s32 *UnitField = &GridEntity->UnitField[RowIndex][ColIndex];
                
                if(Change1DUnitPerSec(UnitSize, ActiveBlockSize, State->GridScalePerSec, TimeElapsed))
                {
                    if(ShouldBreak) ShouldBreak = false;
                }
                else
                {
                    if(IsLevelReady) IsLevelReady = false;
                }
                
                StartY = GridEntity->GridArea.y + (ActiveBlockSize * RowIndex) + (ActiveBlockSize / 2);
                StartX = GridEntity->GridArea.x + (ActiveBlockSize * ColIndex) + (ActiveBlockSize / 2);
                
                AreaQuad.w = *UnitSize;
                AreaQuad.h = *UnitSize;
                AreaQuad.x = StartX - (*UnitSize / 2);
                AreaQuad.y = StartY - (*UnitSize / 2);
                
                if(*UnitField == 0)      GameRenderBitmapToBuffer(Buffer, GridEntity->NormalSquareTexture,     &AreaQuad);
                else if(*UnitField == 2) GameRenderBitmapToBuffer(Buffer, GridEntity->HorizontlaSquareTexture, &AreaQuad);
                else if(*UnitField == 3) GameRenderBitmapToBuffer(Buffer, GridEntity->VerticalSquareTexture,   &AreaQuad);
            }
            
            if(ShouldBreak)
            {
                break;
            }
        }
        
        if(!IsLevelReady) 
        {
            return;
        }
        else
        {
            if(!Change1DUnitPerSec(&FigureEntity->FigureAlpha, 255, State->StartAlphaPerSec, TimeElapsed))
            {
                if(IsLevelReady) IsLevelReady = false;
            }
            
            for(u32 i = 0; i < FigureAmount; ++i)
            {
                SDL_SetTextureAlphaMod(FigureUnit[i].Texture, FigureEntity->FigureAlpha);
                FigureUnitRenderBitmap(Buffer, &FigureUnit[i]);
            }
        }
        
        
        
        if(IsLevelReady)
        {
            State->LevelStarted = IsLevelReady;
            printf("Done!\n");
            printf("TotalElapsed = %f sec\n", (SDL_GetTicks() - TotalElapsed) / 1000.0f);
            
            for(u32 i = 0; i < RowAmount; ++i)
            {
                free(GridEntity->UnitSize[i]);
            }
            
            free(GridEntity->UnitSize);
        }
        
        return;
    }
    
    
    
    //
    // GridEntity Update and Rendering
    //
    for (u32 Index = 0; Index < FigureAmount; ++Index)
    {
        u32 FigureIndex = FigureEntity->FigureOrder[Index];
        bool IsIdle     = FigureUnit[FigureIndex].IsIdle;
        bool IsSticked  = FigureUnit[FigureIndex].IsStick;
        bool IsAttached = FigureIndex == ActiveIndex;
        
        //printf("----------\n");
        //printf("IsIdle = %d\n", IsIdle);
        //printf("IsSticked = %d\n", IsSticked);
        //printf("IsAttached = %d\n", IsAttached);
        
        if(IsIdle)
        {
            continue;
        }
        
        if(IsSticked && IsAttached)
        {
            // Unstick from the grid
            u32 StickAmount = GridEntity->StickUnitsAmount;
            u32 RowIndex = 0;
            u32 ColIndex = 0;
            for (u32 i = 0; i < StickAmount; ++i)
            {
                if(GridEntity->StickUnits[i].Index == FigureIndex)
                {
                    GridEntity->StickUnits[i].Index = -1;
                    for (u32 j = 0; j < 4; ++j)
                    {
                        RowIndex = GridEntity->StickUnits[i].Row[j];
                        ColIndex = GridEntity->StickUnits[i].Col[j];
                        GridEntity->UnitField[RowIndex][ColIndex] = 0;
                    }
                    
                    break;
                }
            }
            
            FigureUnit[FigureIndex].IsStick = false;
        }
        else if(!IsSticked && !IsAttached)
        {
            if(!FigureEntity->IsRotating && !FigureEntity->IsFlipping)
            {
                // Check if we can stick it!
                u32 Count   = 0;
                r32 OffsetX = 0;
                r32 OffsetY = 0;
                u32 RowIndex[4] = {0};
                u32 ColIndex[4] = {0};
                game_rect Rect = {0, 0, (s32)ActiveBlockSize, (s32)ActiveBlockSize};
                
                for (u32 i = 0 ; i < RowAmount && Count != 4; ++i)
                {
                    for (u32 j = 0; j < ColumnAmount && Count != 4; ++j)
                    {
                        Rect.x = GridEntity->GridArea.x + (j*ActiveBlockSize);
                        Rect.y = GridEntity->GridArea.y + (i*ActiveBlockSize);
                        
                        for (u32 l = 0; l < 4; ++l)
                        {
                            if(IsPointInsideRect(FigureUnit[FigureIndex].Shell[l].x,
                                                 FigureUnit[FigureIndex].Shell[l].y,
                                                 &Rect))
                            {
                                if(Count == 0)
                                {
                                    OffsetX = Rect.x + (Rect.w / 2) - FigureUnit[FigureIndex].Shell[l].x;
                                    OffsetY = Rect.y + (Rect.h / 2) - FigureUnit[FigureIndex].Shell[l].y;
                                }
                                
                                RowIndex[l] = i;
                                ColIndex[l] = j;
                                Count = Count + 1;
                                break;
                            }    
                        }
                    }
                }
                
                if(Count == 4)
                {
                    bool IsFree = true;
                    bool IsFull = true;
                    
                    for (u32 i = 0; i < 4; ++i)
                    {
                        if(GridEntity->UnitField[RowIndex[i]][ColIndex[i]] > 0)
                        {
                            IsFree = false;
                        }
                    }
                    
                    if(IsFree)
                    {
                        FigureUnit[FigureIndex].IsStick = true;
                        
                        u32 StickSize = GridEntity->StickUnitsAmount;
                        game_point FigureCenter = FigureUnit[FigureIndex].Center;
                        for (u32 i = 0; i < StickSize; ++i)
                        {
                            if(GridEntity->StickUnits[i].Index == -1)
                            {
                                GridEntity->StickUnits[i].Index     = FigureIndex;
                                GridEntity->StickUnits[i].Center.x  = FigureCenter.x + OffsetX;
                                GridEntity->StickUnits[i].Center.y  = FigureCenter.y + OffsetY;
                                GridEntity->StickUnits[i].IsSticked = false;
                                
                                for (u32 j = 0; j < 4; ++j)
                                {
                                    GridEntity->StickUnits[i].Row[j] = RowIndex[j];
                                    GridEntity->StickUnits[i].Col[j] = ColIndex[j];
                                }
                                
                                break;
                            }
                        }
                        
                        
                        FigureEntityLowPriority(FigureEntity, FigureIndex);
                        
                        for (u32 i = 0; i < RowAmount; ++i)
                        {
                            for (u32 j = 0; j < ColumnAmount; ++j)
                            {
                                if(GridEntity->UnitField[i][j] == 0)
                                {
                                    IsFull = true;
                                }
                            }
                        }
                        
                        
                        
                        if(IsFull == true)
                        {
                            // the grid is full and level is complete 
                        }
                    }
                }
            }
        }
    }
    
    
    for(u32 i = 0; i < GridEntity->StickUnitsAmount; ++i)
    {
        s32 Index = GridEntity->StickUnits[i].Index;
        if(!GridEntity->StickUnits[i].IsSticked && Index >= 0)
        {
            game_point *FigureCenter = &FigureUnit[Index].Center;
            game_point *TargetCenter = &GridEntity->StickUnits[i].Center;
            
            vector2 Velocity = Move2DPointPerSec(FigureCenter, TargetCenter, MaxVel, TimeElapsed);
            
            FigureUnitMove(&FigureUnit[Index], Velocity.x, Velocity.y);
            
            if((FigureCenter->x == TargetCenter->x) && (FigureCenter->y == TargetCenter->y))
            {
                GridEntity->StickUnits[i].IsSticked = true;
                
                u32 RowIndex = 0;
                u32 ColIndex = 0;
                
                for(u32 j = 0; j < 4; j++)
                {
                    RowIndex = GridEntity->StickUnits[i].Row[j];
                    ColIndex = GridEntity->StickUnits[i].Col[j];
                    GridEntity->UnitField[RowIndex][ColIndex] = 1;
                }
            }
        }
    }
    
    AreaQuad.w = ActiveBlockSize;
    AreaQuad.h = ActiveBlockSize;
    
    for (u32 i = 0; i < RowAmount; ++i)
    {
        StartY = GridEntity->GridArea.y + (ActiveBlockSize * i) + (ActiveBlockSize / 2);
        for (u32 j = 0; j < ColumnAmount; ++j)
        {
            StartX = GridEntity->GridArea.x + (ActiveBlockSize * j) + (ActiveBlockSize / 2);
            
            AreaQuad.x = StartX - (AreaQuad.w / 2);
            AreaQuad.y = StartY - (AreaQuad.h / 2);
            
            u32 GridUnit = GridEntity->UnitField[i][j];
            if(GridUnit == 0 || GridUnit == 2 || GridUnit == 3)
            {
                GameRenderBitmapToBuffer(Buffer, GridEntity->NormalSquareTexture, &AreaQuad);
            }
        }
    }
    
    //
    // MovingBlocks Update and Rendering
    //
    
    for(u32 i = 0; i < GridEntity->MovingBlocksAmount; ++i)
    {
        if(GridEntity->MovingBlocks[i].IsMoving)
        {
            s32 RowNumber = GridEntity->MovingBlocks[i].RowNumber;
            s32 ColNumber = GridEntity->MovingBlocks[i].ColNumber;
            
            game_point Center = { 
                GridEntity->MovingBlocks[i].AreaQuad.x, 
                GridEntity->MovingBlocks[i].AreaQuad.y 
            };
            
            game_point TargetCenter;
            TargetCenter.x = GridEntity->GridArea.x + (ColNumber * ActiveBlockSize);
            TargetCenter.y = GridEntity->GridArea.y + (RowNumber * ActiveBlockSize);
            
            vector2 Velocity = Move2DPointPerSec(&Center, &TargetCenter, MaxVel, TimeElapsed);
            
            GridEntity->MovingBlocks[i].AreaQuad.x += Velocity.x;
            GridEntity->MovingBlocks[i].AreaQuad.y += Velocity.y;
            
            if((Center.x == TargetCenter.x) && (Center.y == TargetCenter.y))
            {
                GridEntity->MovingBlocks[i].IsMoving = false;
                GridEntity->MovingBlocks[i].AreaQuad.x
                    = GridEntity->GridArea.x + (ColNumber * ActiveBlockSize);
                GridEntity->MovingBlocks[i].AreaQuad.y
                    = GridEntity->GridArea.y + (RowNumber * ActiveBlockSize);
                GridEntity->UnitField[RowNumber][ColNumber] = 1;
            }
        }
        
        if(GridEntity->MovingBlocks[i].IsVertical)
        {
            GameRenderBitmapToBuffer(Buffer, GridEntity->VerticalSquareTexture,   &GridEntity->MovingBlocks[i].AreaQuad);
        }
        else
        {
            GameRenderBitmapToBuffer(Buffer, GridEntity->HorizontlaSquareTexture, &GridEntity->MovingBlocks[i].AreaQuad);
            }
    }
    
    //
    // FigureEntity Update and Rendering
    //
    
    //
    // Figure Area Highlight
    //
    
    if(FigureEntity->IsRestarting)
    {
        bool AllFiguresReturned = true;
        for(u32 i = 0; i < FigureAmount; ++i)
        {
            game_point *Center        = &FigureUnit[i].Center;
            game_point *TargetCenter  = &FigureUnit[i].DefaultCenter;
            if(!FigureUnit[i].IsIdle)
            {
                AllFiguresReturned = false;
                
                vector2 Velocity =  Move2DPointPerSec(Center,TargetCenter, MaxVel, TimeElapsed);
                FigureUnitMove(&FigureUnit[i], Velocity.x, Velocity.y);
                
                if((Center->x == TargetCenter->x) && (Center->y == TargetCenter->y))
                {
                    FigureEntity->FigureUnit[i].IsIdle = true;
                }
            }
        }
        
        if(AllFiguresReturned)
        {
            FigureEntity->IsRestarting = false;
            printf("Restarted!\n");
            }
    }
    
    bool ShouldHighlight = false;
    
    if(ActiveIndex >= 0)
    {
        game_rect ScreenArea = {0};
        ScreenArea.w = Buffer->Width;
        ScreenArea.h = Buffer->Height;
        
        ShouldHighlight = IsFigureUnitInsideRect(&FigureUnit[ActiveIndex], &FigureEntity->FigureArea) || !(IsPointInsideRect(FigureUnit[ActiveIndex].Center.x,  FigureUnit[ActiveIndex].Center.y, &ScreenArea));
        
    }
    
    if(ShouldHighlight) Change1DUnitPerSec(&FigureEntity->AreaAlpha, 255, State->FlippingAlphaPerSec, TimeElapsed);
    else                Change1DUnitPerSec(&FigureEntity->AreaAlpha, 0, State->FlippingAlphaPerSec, TimeElapsed);
    
    if(FigureEntity->AreaAlpha != 0) ToggleHighlight = true;
    
    if(ToggleHighlight) FigureEntityHighlightFigureArea(FigureEntity, Buffer, {255, 255, 255}, InActiveBlockSize / 6);
    
    //
    // Figure returning to the idle zone
    //
    
    
    
    if(FigureEntity->IsReturning)
    {
        u32 ReturnIndex = FigureEntity->ReturnIndex;
        game_point *Center        = &FigureUnit[ReturnIndex].Center;
        game_point *TargetCenter  = &FigureUnit[ReturnIndex].DefaultCenter;
        
        vector2 Velocity =  Move2DPointPerSec(Center,TargetCenter, MaxVel, TimeElapsed);
        
        FigureUnitMove(&FigureUnit[ReturnIndex], Velocity.x, Velocity.y);
        
        if((Center->x == TargetCenter->x) && (Center->y == TargetCenter->y))
        {
            FigureEntity->IsReturning = false;
            FigureEntity->ReturnIndex = -1;
        }
    }
    
    //
    // Rotation Animation
    //
    
    if(FigureEntity->IsRotating)
    {
        // TODO(max): Maybe put these values in level_entity ???
        r32 AngleDt = TimeElapsed * State->RotationVel;
        if(FigureEntity->RotationSum < 90.0f && !(FigureEntity->RotationSum + AngleDt >= 90.0f))
        {
            FigureEntity->FigureUnit[ActiveIndex].Angle += AngleDt;
            FigureEntity->RotationSum += AngleDt;
        }
        else
        {
            FigureEntity->FigureUnit[ActiveIndex].Angle += 90.0f - FigureEntity->RotationSum;
            FigureEntity->RotationSum = 0;
            FigureEntity->IsRotating = false;
        }
    }
    
    //
    // Flipping Animation
    //
    
    if(FigureEntity->IsFlipping)
    {
        if(FigureEntity->FadeInSum > 0)
        {
            if(Change1DUnitPerSec(&FigureEntity->FigureAlpha, 0, State->FlippingAlphaPerSec, TimeElapsed))
            {
                FigureEntity->FigureAlpha = 0;
                FigureEntity->FadeInSum   = 0;
                FigureUnitFlipHorizontally(&FigureUnit[ActiveIndex]);
            }
        }
        else if(FigureEntity->FadeOutSum < 255)
        {
            if(Change1DUnitPerSec(&FigureEntity->FigureAlpha, 255, State->FlippingAlphaPerSec, TimeElapsed))
            {
                FigureEntity->FigureAlpha = 255;
                FigureEntity->FadeOutSum  = 255;
                FigureEntity->IsFlipping  = false;
            }
        }
        
        SDL_SetTextureAlphaMod(FigureUnit[ActiveIndex].Texture, FigureEntity->FigureAlpha);
    }
    
    for(u32 i = 0; i < FigureAmount; ++i)
    {
        u32 Index = FigureEntity->FigureOrder[i];
        //u32 BlockSize = FigureUnit[Index].IsEnlarged ? ActiveBlockSize : InActiveBlockSize;
        //DEBUGRenderFigureShell(Buffer, &FigureUnit[Index], InActiveBlockSize, {255, 255, 0});
        FigureUnitRenderBitmap(Buffer, &FigureUnit[Index]);
        
        //DEBUGRenderQuad(Buffer, &FigureUnit[Index].AreaQuad, {255, 0, 0}, 255);
    }
    
}


static void
RescaleGameField(game_offscreen_buffer *Buffer,
                 u32 RowAmount, u32 ColumnAmount, u32 FigureAmount,
                 u32 DefaultBlocksInRow, u32 DefaultBlocksInCol,
                 level_entity *LevelEntity)
{
    u32 InActiveBlockSize  = 0;
    u32 ActiveBlockSize    = 0;
    
    u32 FigureAreaWidth  = Buffer->Width;
    u32 FigureAreaHeight = Buffer->Height * 0.4f;
    
    u32 BlocksInRow = (FigureAmount / 2.0) + 0.5f;
    BlocksInRow     = (BlocksInRow * 2) + 2;
    
    InActiveBlockSize = GameResizeInActiveBlock(FigureAreaWidth, FigureAreaHeight,
                                                DefaultBlocksInRow, DefaultBlocksInCol,BlocksInRow);
    FigureAreaHeight = InActiveBlockSize * DefaultBlocksInCol;
    
    u32 GridAreaWidth  = Buffer->Width;
    u32 GridAreaHeight = Buffer->Height - FigureAreaHeight;
    
    ActiveBlockSize = GameResizeActiveBlock(GridAreaWidth, GridAreaHeight, RowAmount, ColumnAmount);
    
    LevelEntity->ActiveBlockSize   = ActiveBlockSize;
    LevelEntity->InActiveBlockSize = InActiveBlockSize;
}

static void
LevelEditorInit(level_entity *LevelEntity, game_memory *Memory, game_offscreen_buffer *Buffer)
{
    Memory->LevelEditor = (level_editor*)malloc(sizeof(level_editor));
    Assert(Memory->LevelEditor);
    
    u32 ButtonSize   = LevelEntity->InActiveBlockSize * 2;
    u32 RowAmount    = LevelEntity->GridEntity->RowAmount;
    u32 ColumnAmount = LevelEntity->GridEntity->ColumnAmount;
    
    char RowString[2] = {0};
    char ColString[2] = {0};
    sprintf(RowString, "%d", RowAmount);
    sprintf(ColString, "%d", ColumnAmount);
    
    level_editor *&LevelEditor = Memory->LevelEditor;
    LevelEditor->GridButtonLayer.w = ButtonSize * 6;
    LevelEditor->GridButtonLayer.h = ButtonSize;
    LevelEditor->GridButtonLayer.x = (Buffer->Width / 2) - (LevelEditor->GridButtonLayer.w / 2);
    LevelEditor->GridButtonLayer.y = LevelEntity->FigureEntity->FigureArea.y - ButtonSize;
    
    LevelEditor->Font = TTF_OpenFont("..\\data\\Karmina-Bold.otf", ButtonSize);
    Assert(LevelEditor->Font);
    
    game_surface *Surface = TTF_RenderUTF8_Blended(LevelEditor->Font, "+", {0, 0, 0});
    LevelEditor->PlusTexture = SDL_CreateTextureFromSurface(Buffer->Renderer, Surface);
    SDL_QueryTexture(LevelEditor->PlusTexture, 0, 0, &LevelEditor->GridButtonQuad[0].w, &LevelEditor->GridButtonQuad[0].h);
    SDL_QueryTexture(LevelEditor->PlusTexture, 0, 0, &LevelEditor->GridButtonQuad[3].w, &LevelEditor->GridButtonQuad[3].h);
    SDL_FreeSurface(Surface);
    
    
    Surface = TTF_RenderUTF8_Blended(LevelEditor->Font, "-", {0, 0, 0});
    LevelEditor->MinusTexture = SDL_CreateTextureFromSurface(Buffer->Renderer, Surface);
    SDL_QueryTexture(LevelEditor->MinusTexture, 0, 0, &LevelEditor->GridButtonQuad[2].w,
                     &LevelEditor->GridButtonQuad[2].h);
    SDL_QueryTexture(LevelEditor->MinusTexture, 0, 0, &LevelEditor->GridButtonQuad[5].w,
                     &LevelEditor->GridButtonQuad[5].h);
    SDL_FreeSurface(Surface);
    
    Surface = TTF_RenderUTF8_Blended(LevelEditor->Font, RowString, {0, 0, 0});
    LevelEditor->RowTexture = SDL_CreateTextureFromSurface(Buffer->Renderer, Surface);
    SDL_QueryTexture(LevelEditor->RowTexture, 0, 0, &LevelEditor->GridButtonQuad[1].w,
                     &LevelEditor->GridButtonQuad[1].h);
    SDL_FreeSurface(Surface);
    
    Surface = TTF_RenderUTF8_Blended(LevelEditor->Font, ColString, {0, 0, 0});
    LevelEditor->ColumnTexture = SDL_CreateTextureFromSurface(Buffer->Renderer, Surface);
    SDL_QueryTexture(LevelEditor->ColumnTexture, 0, 0, &LevelEditor->GridButtonQuad[4].w, &LevelEditor->GridButtonQuad[4].h);
    SDL_FreeSurface(Surface);
    
    
    game_rect UiQuad = {LevelEditor->GridButtonLayer.x, LevelEditor->GridButtonLayer.y, 
        ButtonSize, ButtonSize
    };
    
    for(u32 i = 0; i < 6; i++)
    {
        LevelEditor->GridButtonQuad[i].x = (UiQuad.x + UiQuad.w / 2) - (LevelEditor->GridButtonQuad[i].w / 2);
        LevelEditor->GridButtonQuad[i].y = (UiQuad.y + UiQuad.h / 2) - (LevelEditor->GridButtonQuad[i].h / 2);
        UiQuad.x += UiQuad.w;
    }
    
    LevelEditor->FigureButtonLayer.w = ButtonSize * 6;
    LevelEditor->FigureButtonLayer.h = ButtonSize;
    LevelEditor->FigureButtonLayer.x = (Buffer->Width / 2) - (LevelEditor->FigureButtonLayer.w / 2);
    LevelEditor->FigureButtonLayer.y = Buffer->Height - LevelEditor->FigureButtonLayer.h;
    
    LevelEditor->FigureButtonQuad[0].w = LevelEditor->GridButtonQuad[0].w;
    LevelEditor->FigureButtonQuad[0].h = LevelEditor->GridButtonQuad[0].h;
    LevelEditor->FigureButtonQuad[1].w = LevelEditor->GridButtonQuad[2].w;
    LevelEditor->FigureButtonQuad[1].h = LevelEditor->GridButtonQuad[2].h;
    
    Surface = TTF_RenderUTF8_Blended(LevelEditor->Font, "R", {0, 0, 0});
    LevelEditor->RotateTexture  = SDL_CreateTextureFromSurface(Buffer->Renderer, Surface);
    SDL_QueryTexture(LevelEditor->RotateTexture, 0, 0, &LevelEditor->FigureButtonQuad[2].w, &LevelEditor->FigureButtonQuad[2].h);
    SDL_FreeSurface(Surface);
    
    Surface = TTF_RenderUTF8_Blended(LevelEditor->Font, "F", {0, 0, 0});
    LevelEditor->FlipTexture  = SDL_CreateTextureFromSurface(Buffer->Renderer, Surface);
    SDL_QueryTexture(LevelEditor->FlipTexture, 0, 0, &LevelEditor->FigureButtonQuad[3].w, &LevelEditor->FigureButtonQuad[3].h);
    SDL_FreeSurface(Surface);
    
    Surface = TTF_RenderUTF8_Blended(LevelEditor->Font, "<", {0, 0, 0});
    LevelEditor->FormTexture  = SDL_CreateTextureFromSurface(Buffer->Renderer, Surface);
    SDL_QueryTexture(LevelEditor->FormTexture, 0, 0, &LevelEditor->FigureButtonQuad[4].w, &LevelEditor->FigureButtonQuad[4].h);
    SDL_FreeSurface(Surface);
    
    Surface = TTF_RenderUTF8_Blended(LevelEditor->Font, ">", {0, 0, 0});
    LevelEditor->TypeTexture  = SDL_CreateTextureFromSurface(Buffer->Renderer, Surface);
    SDL_QueryTexture(LevelEditor->TypeTexture, 0, 0, &LevelEditor->FigureButtonQuad[5].w, &LevelEditor->FigureButtonQuad[5].h);
    SDL_FreeSurface(Surface);
    
    UiQuad.x = LevelEditor->FigureButtonLayer.x;
    UiQuad.y = LevelEditor->FigureButtonLayer.y;
    UiQuad.w = ButtonSize;
    UiQuad.h = ButtonSize;
    
    for(u32 i = 0; i < 6; i++)
    {
        LevelEditor->FigureButtonQuad[i].x = (UiQuad.x + UiQuad.w / 2) - (LevelEditor->FigureButtonQuad[i].w / 2);
        LevelEditor->FigureButtonQuad[i].y = (UiQuad.y + UiQuad.h / 2) - (LevelEditor->FigureButtonQuad[i].h / 2);
        UiQuad.x += UiQuad.w;
    }
}

static void
GridEntityDeleteMovingBlock(grid_entity *GridEntity, u32 Index)
{
    u32 Amount = GridEntity->MovingBlocksAmount;
    
    if(Index < 0 || Index >= Amount) return;
    if(Amount <= 0) return;
    
    moving_block *MovingBlocks = GridEntity->MovingBlocks;
    
    for(u32 i = Index; i < Amount-1; ++i)
    {
        MovingBlocks[i].AreaQuad   = MovingBlocks[i+1].AreaQuad;
        MovingBlocks[i].RowNumber  = MovingBlocks[i+1].RowNumber;
        MovingBlocks[i].ColNumber  = MovingBlocks[i+1].ColNumber;
        MovingBlocks[i].IsVertical = MovingBlocks[i+1].IsVertical;
        MovingBlocks[i].IsMoving   = MovingBlocks[i+1].IsMoving;
        MovingBlocks[i].MoveSwitch = MovingBlocks[i+1].MoveSwitch;
        }
        
        GridEntity->MovingBlocksAmount -= 1;
}

static void
GridEntityNewGrid(game_offscreen_buffer *Buffer, level_entity *LevelEntity, 
                  s32 NewRowAmount, s32 NewColumnAmount, level_editor *LevelEditor)
{
    if(NewRowAmount < 0 || NewColumnAmount < 0) return;
    
    grid_entity *&GridEntity = LevelEntity->GridEntity;
    
    s32 **UnitField = (s32**)malloc(sizeof(s32*) * NewRowAmount);
    Assert(UnitField);
    for(u32 i = 0; i < NewRowAmount; ++i){
        UnitField[i] = (s32*)malloc(sizeof(s32) * NewColumnAmount);
        Assert(UnitField[i]);
        for(u32 j = 0; j < NewColumnAmount; j ++){
            UnitField[i][j] = 0;
        }
    }
    
    u32 CurrentRowAmount = NewRowAmount < GridEntity->RowAmount ? NewRowAmount : GridEntity->RowAmount;
    u32 CurrentColumnAmount = NewColumnAmount < GridEntity->ColumnAmount ? NewColumnAmount : GridEntity->ColumnAmount;
    
    for(u32 i = 0; i < CurrentRowAmount; ++i){
        for(u32 j = 0; j < CurrentColumnAmount; ++j){
            UnitField[i][j] = GridEntity->UnitField[i][j];
        }
    }
    
    for(u32 i = 0; i < GridEntity->RowAmount; ++i){
        free(GridEntity->UnitField[i]);
    }
    
    free(GridEntity->UnitField);
    
    game_surface *Surface = 0;
    char RowString[2] = {0};
    char ColString[2] = {0};
    
    if(NewRowAmount != GridEntity->RowAmount)
    {
        sprintf(RowString, "%d", NewRowAmount);
        
        Surface = TTF_RenderUTF8_Blended(LevelEditor->Font, RowString, { 0, 0, 0 });
        LevelEditor->RowTexture = SDL_CreateTextureFromSurface(Buffer->Renderer, Surface);
        SDL_QueryTexture(LevelEditor->RowTexture, 0, 0, &LevelEditor->GridButtonQuad[1].w, &LevelEditor->GridButtonQuad[1].h);
        SDL_FreeSurface(Surface);
    }
    else if(NewColumnAmount != GridEntity->ColumnAmount)
    {
        sprintf(ColString, "%d", NewColumnAmount);
        
        Surface = TTF_RenderUTF8_Blended(LevelEditor->Font, ColString, { 0, 0, 0 });
        LevelEditor->ColumnTexture = SDL_CreateTextureFromSurface(Buffer->Renderer, Surface);
        SDL_QueryTexture(LevelEditor->ColumnTexture, 0, 0, &LevelEditor->GridButtonQuad[4].w, &LevelEditor->GridButtonQuad[4].h);
        SDL_FreeSurface(Surface);
    }
    
    u32 DefaultBlocksInRow = 12;
    u32 DefaultBlocksInCol = 9;
    
    RescaleGameField(Buffer, NewRowAmount, NewColumnAmount, LevelEntity->FigureEntity->FigureAmount, DefaultBlocksInRow, DefaultBlocksInCol, LevelEntity);
    
    GridEntity->UnitField    = UnitField;
    GridEntity->RowAmount    = NewRowAmount;
    GridEntity->ColumnAmount = NewColumnAmount;
    
    u32 ActiveBlockSize   = LevelEntity->ActiveBlockSize;
    u32 InActiveBlockSize = LevelEntity->InActiveBlockSize;
    
    LevelEntity->GridEntity->GridArea.w = ActiveBlockSize * NewColumnAmount;
    LevelEntity->GridEntity->GridArea.h = ActiveBlockSize * NewRowAmount;
    LevelEntity->GridEntity->GridArea.x = (Buffer->Width / 2) - (GridEntity->GridArea.w / 2);
    LevelEntity->GridEntity->GridArea.y = (Buffer->Height - LevelEntity->FigureEntity->FigureArea.h) / 2 - (GridEntity->GridArea.h / 2);
    
}

static void
LevelEditorUpdateAndRender(level_editor *LevelEditor, level_entity *LevelEntity, 
                           game_offscreen_buffer *Buffer, game_input *Input)
{
    if(Input->WasPressed)
    {
        if(Input->BackQuote.IsDown)
        {
            printf("BackQuote!\n");
            if(!LevelEntity->LevelPaused)
            {
                LevelEntity->LevelPaused = true;
                RestartLevelEntity(LevelEntity);
            }
            else
            {
                LevelEntity->LevelPaused = false;
            }
        }
    }
    
    if(!LevelEntity->LevelPaused) return;
    
    game_rect GridArea = LevelEntity->GridEntity->GridArea;
    
    u32 RowAmount  = LevelEntity->GridEntity->RowAmount;
    u32 ColAmount  = LevelEntity->GridEntity->ColumnAmount;
    
    if(Input->WasPressed){
        
        if(Input->LeftClick.IsDown)
        {
            if(IsPointInsideRect(Input->MouseX, Input->MouseY, &LevelEditor->GridButtonLayer))
            {
                /* Grid layer*/
                
                /* Plus row */
                if (IsPointInsideRect(Input->MouseX, Input->MouseY,
                                      &LevelEditor->GridButtonQuad[0]))
                {
                    printf("Plus row!\n");
                    GridEntityNewGrid(Buffer, LevelEntity, RowAmount+1, ColAmount, LevelEditor);
                }
                /* Minus row */
                else if(IsPointInsideRect(Input->MouseX, Input->MouseY, 
                                          &LevelEditor->GridButtonQuad[2]))
                {
                    printf("Minus row!\n");
                    GridEntityNewGrid(Buffer, LevelEntity, RowAmount-1, ColAmount, LevelEditor);
                }
                /* Plus column */
                else if(IsPointInsideRect(Input->MouseX, Input->MouseY, 
                                          &LevelEditor->GridButtonQuad[3]))
                {
                    printf("Plus column!\n");
                    GridEntityNewGrid(Buffer, LevelEntity, RowAmount, ColAmount+1, LevelEditor);
                }
                /* Minus column */
                else if(IsPointInsideRect(Input->MouseX, Input->MouseY, 
                                          &LevelEditor->GridButtonQuad[5]))
                {
                    printf("Minus column!\n");
                    GridEntityNewGrid(Buffer, LevelEntity, RowAmount, ColAmount-1, LevelEditor);
                }
            }
            else if(IsPointInsideRect(Input->MouseX, Input->MouseY, &LevelEditor->FigureButtonLayer))
            {
                /* Figure Layer*/ 
                
                /* Add figure*/ 
                if (IsPointInsideRect(Input->MouseX, Input->MouseY,
                                      &LevelEditor->FigureButtonQuad[0]))
                {
                    
                }
                /* Delete figure */
                else if(IsPointInsideRect(Input->MouseX, Input->MouseY, 
                                          &LevelEditor->FigureButtonQuad[1]))
                {
                    
                }
                /* Rotate figure */
                else if(IsPointInsideRect(Input->MouseX, Input->MouseY, 
                                          &LevelEditor->FigureButtonQuad[2]))
                {
                    
                }
                /* Flip figure */
                else if(IsPointInsideRect(Input->MouseX, Input->MouseY, 
                                          &LevelEditor->FigureButtonQuad[3]))
                {
                    
                }
                /* Change figure form */
                else if(IsPointInsideRect(Input->MouseX, Input->MouseY, 
                                          &LevelEditor->FigureButtonQuad[4]))
                {
                    
                }
                /* Change figure type */
                else if(IsPointInsideRect(Input->MouseX, Input->MouseY, 
                                          &LevelEditor->FigureButtonQuad[5]))
                {
                    
                }
            }
            else if(IsPointInsideRect(Input->MouseX, Input->MouseY, &GridArea))
            {
                printf("GridArea click !\n");
                
                game_rect AreaQuad = { 0, 0, LevelEntity->ActiveBlockSize, LevelEntity->ActiveBlockSize };
                
                u32 StartX = 0;
                u32 StartY = 0;
                
                for(u32 i = 0; i < RowAmount; ++i)
                {
                    StartY = GridArea.y + (LevelEntity->ActiveBlockSize * i);
                    
                    for(u32 j = 0; j < ColAmount; ++j)
                    {
                        StartX = GridArea.x + (LevelEntity->ActiveBlockSize * j);
                        
                        AreaQuad.x = StartX;
                        AreaQuad.y = StartY;
                        
                        if(IsPointInsideRect(Input->MouseX, Input->MouseY, 
                                             &AreaQuad))
                        {
                            u32 GridUnit = LevelEntity->GridEntity->UnitField[i][j];
                            if(GridUnit == 0)
                            {
                                LevelEntity->GridEntity->UnitField[i][j] = 1;
}
                            else
                            {
                                s32 Index = -1;
                                for(u32 m = 0; m < LevelEntity->GridEntity->MovingBlocksAmount; ++m)
                                {
                                    u32 RowNumber = LevelEntity->GridEntity->MovingBlocks[m].RowNumber;
                                    u32 ColNumber = LevelEntity->GridEntity->MovingBlocks[m].ColNumber;
                                    
                                    if((i == RowNumber) && (j == ColNumber))
                                    {
                                        Index = m;
                                        break;
                                    }
                                }
                                
                                if(Index >= 0)
                                {
                                    
                                    if(!LevelEntity->GridEntity->MovingBlocks[Index].MoveSwitch)
                                    {
                                        LevelEntity->GridEntity->MovingBlocks[Index].MoveSwitch = true;
                                        }
                                    else if(!LevelEntity->GridEntity->MovingBlocks[Index].IsVertical)
                                    {
                                        LevelEntity->GridEntity->MovingBlocks[Index].IsVertical = true;
                                        LevelEntity->GridEntity->MovingBlocks[Index].MoveSwitch = false;
                                    }
                                    else
                                    {
                                        GridEntityDeleteMovingBlock(LevelEntity->GridEntity, Index);
                                        LevelEntity->GridEntity->UnitField[i][j] = 0;
                                    }
                                }
                                else
                                {
                                    GridEntityAddMovingBlock(LevelEntity->GridEntity, i, j, false, false, LevelEntity->ActiveBlockSize);
                                }
                            }
                        }
                        }
                }
            }
        }
    }
    
    game_rect ButtonQuad = 
    {
        ButtonQuad.x = LevelEditor->GridButtonLayer.x,
        ButtonQuad.y = LevelEditor->GridButtonLayer.y,
        ButtonQuad.w = LevelEntity->InActiveBlockSize * 2, 
        ButtonQuad.h = LevelEntity->InActiveBlockSize * 2
    };
    
    DEBUGRenderQuadFill(Buffer, &LevelEditor->GridButtonLayer, {0, 0, 255}, 255);
    
    DEBUGRenderQuad(Buffer, &ButtonQuad, {0, 0, 0}, 255);
    GameRenderBitmapToBuffer(Buffer, LevelEditor->PlusTexture, &LevelEditor->GridButtonQuad[0]);
    
    ButtonQuad.x += ButtonQuad.w;
    
    DEBUGRenderQuad(Buffer, &ButtonQuad, {0, 0, 0}, 255);
    GameRenderBitmapToBuffer(Buffer, LevelEditor->RowTexture, &LevelEditor->GridButtonQuad[1]);
    
    ButtonQuad.x += ButtonQuad.w;
    
    DEBUGRenderQuad(Buffer, &ButtonQuad, {0, 0, 0}, 255);
    GameRenderBitmapToBuffer(Buffer, LevelEditor->MinusTexture,  &LevelEditor->GridButtonQuad[2]);
    
    ButtonQuad.x += ButtonQuad.w;
    
    DEBUGRenderQuad(Buffer, &ButtonQuad, {0, 0, 0}, 255);
    GameRenderBitmapToBuffer(Buffer, LevelEditor->PlusTexture, &LevelEditor->GridButtonQuad[3]);
    
    ButtonQuad.x += ButtonQuad.w;
    
    DEBUGRenderQuad(Buffer, &ButtonQuad, {0, 0, 0}, 255);
    GameRenderBitmapToBuffer(Buffer, LevelEditor->ColumnTexture, &LevelEditor->GridButtonQuad[4]);
    
    ButtonQuad.x += ButtonQuad.w;
    
    DEBUGRenderQuad(Buffer, &ButtonQuad, {0, 0, 0}, 255);
    GameRenderBitmapToBuffer(Buffer, LevelEditor->MinusTexture, &LevelEditor->GridButtonQuad[5]);
    
    
    ButtonQuad.x = LevelEditor->FigureButtonLayer.x;
    ButtonQuad.y = LevelEditor->FigureButtonLayer.y;
    
    DEBUGRenderQuadFill(Buffer, &LevelEditor->FigureButtonLayer, {0, 255, 0}, 255);
    
    DEBUGRenderQuad(Buffer, &ButtonQuad, {0, 0, 0}, 255);
    GameRenderBitmapToBuffer(Buffer, LevelEditor->PlusTexture, 
                             &LevelEditor->FigureButtonQuad[0]);
    
    ButtonQuad.x += ButtonQuad.w;
    
    DEBUGRenderQuad(Buffer, &ButtonQuad, {0, 0, 0}, 255);
    GameRenderBitmapToBuffer(Buffer, LevelEditor->MinusTexture, 
                             &LevelEditor->FigureButtonQuad[1]);
    
    ButtonQuad.x += ButtonQuad.w;
    
    DEBUGRenderQuad(Buffer, &ButtonQuad, {0, 0, 0}, 255);
    GameRenderBitmapToBuffer(Buffer, LevelEditor->RotateTexture, 
                             &LevelEditor->FigureButtonQuad[2]);
    
    ButtonQuad.x += ButtonQuad.w;
    
    DEBUGRenderQuad(Buffer, &ButtonQuad, {0, 0, 0}, 255);
    GameRenderBitmapToBuffer(Buffer, LevelEditor->FlipTexture, 
                             &LevelEditor->FigureButtonQuad[3]);
    
    ButtonQuad.x += ButtonQuad.w;
    
    DEBUGRenderQuad(Buffer, &ButtonQuad, {0, 0, 0}, 255);
    GameRenderBitmapToBuffer(Buffer, LevelEditor->FormTexture, 
                             &LevelEditor->FigureButtonQuad[4]);
    
    ButtonQuad.x += ButtonQuad.w;
    
    DEBUGRenderQuad(Buffer, &ButtonQuad, {0, 0, 0}, 255);
    GameRenderBitmapToBuffer(Buffer, LevelEditor->TypeTexture, 
                             &LevelEditor->FigureButtonQuad[5]);
}


static bool
GameUpdateAndRender(game_memory *Memory, game_input *Input, game_offscreen_buffer *Buffer)
{
    static r32 PreviousTimeTick = 0;
    static r32 CurrentTimeTick  = 0;
    
    bool ShouldQuit       = false;
    
    level_entity  *GameState     = &Memory->LevelEntity;
    grid_entity   *&GridEntity   = GameState->GridEntity;
    figure_entity *&FigureEntity = GameState->FigureEntity;
    
    if(!Memory->IsInitialized)
    {
        u32 DefaultBlocksInRow = 12;
        u32 DefaultBlocksInCol = 9;
        
        u32 RowAmount           = 5;
        u32 ColumnAmount        = 5;
        u32 FigureAmountReserve = 20;
        u32 MovingBlocksAmountReserved  = 10;
        
        GameState->LevelStarted  = false;
        GameState->LevelFinished = false;
        
        RescaleGameField(Buffer, RowAmount, ColumnAmount, FigureAmountReserve, DefaultBlocksInRow, DefaultBlocksInCol, GameState);
        
        u32 ActiveBlockSize   = GameState->ActiveBlockSize;
        u32 InActiveBlockSize = GameState->InActiveBlockSize;
        
        //
        // Figure initialization
        //
        FigureEntity = (figure_entity*)malloc(sizeof(figure_entity));
        Assert(FigureEntity);
        
        FigureEntity->FigureAmount         = 0;
        FigureEntity->FigureAmountReserved = FigureAmountReserve;
        
        FigureEntity->ReturnIndex   = -1;
        FigureEntity->FigureActive  = -1;
        
        FigureEntity->IsGrabbed     = false;
        FigureEntity->IsRotating    = false;
        FigureEntity->IsReturning   = false;
        FigureEntity->IsRestarting  = false;
        FigureEntity->IsFlipping    = false;
        
        FigureEntity->RotationSum   = 0;
        FigureEntity->AreaAlpha     = 0;
        FigureEntity->FigureAlpha   = 0;
        FigureEntity->FadeInSum     = 0;
        FigureEntity->FadeOutSum    = 0;
        
        FigureEntity->FigureArea.w  = Buffer->Width;
        FigureEntity->FigureArea.h  = InActiveBlockSize * DefaultBlocksInCol;
        FigureEntity->FigureArea.y  = Buffer->Height - (FigureEntity->FigureArea.h);
        FigureEntity->FigureArea.x  = 0;
        
        FigureEntity->FigureUnit = (figure_unit*)malloc(sizeof(figure_unit)*FigureEntity->FigureAmountReserved);
        Assert(FigureEntity->FigureUnit);
        
        FigureUnitAddNewFigure(FigureEntity, "z_d.png", Z_figure, classic, Memory, Buffer);
        FigureUnitAddNewFigure(FigureEntity, "i_m.png", I_figure, classic, Memory, Buffer);
        FigureUnitAddNewFigure(FigureEntity, "j_s.png", J_figure, classic, Memory, Buffer);
        
        FigureEntity->FigureOrder = (u32*)malloc(sizeof(u32) * FigureEntity->FigureAmountReserved);
        Assert(FigureEntity->FigureOrder);
        
        for(u32 i = 0; i < FigureEntity->FigureAmountReserved; ++i) FigureEntity->FigureOrder[i] = i;
        
        // Grid initialization
        GridEntity  = (grid_entity *) malloc(sizeof(grid_entity));
        Assert(GridEntity);
        
        GridEntity->RowAmount           = RowAmount;
        GridEntity->ColumnAmount        = ColumnAmount;
        GridEntity->StickUnitsAmount    = FigureEntity->FigureAmount;
        GridEntity->MovingBlocksAmount  = 0;
        GridEntity->MovingBlocksAmountReserved  = MovingBlocksAmountReserved;
        GridEntity->GridArea.w = ActiveBlockSize * ColumnAmount;
        GridEntity->GridArea.h = ActiveBlockSize * RowAmount;
        GridEntity->GridArea.x = (Buffer->Width / 2) - (GridEntity->GridArea.w / 2);
        GridEntity->GridArea.y = (Buffer->Height - FigureEntity->FigureArea.h) / 2 - (GridEntity->GridArea.h / 2);
        
        //
        // UnitField initialization
        //
        GridEntity->UnitField = (s32**)malloc(GridEntity->RowAmount * sizeof(s32*));
        Assert(GridEntity->UnitField);
        for (u32 i = 0; i < GridEntity->RowAmount; ++i)
        {
            GridEntity->UnitField[i] = (s32*)malloc(sizeof(s32) * GridEntity->ColumnAmount);
            Assert(GridEntity->UnitField[i]);
            for (u32 j = 0; j < GridEntity->ColumnAmount; ++j)
            {
                GridEntity->UnitField[i][j] = 0;
            }
        }
        
        GridEntity->UnitSize = (r32**)malloc(GridEntity->RowAmount * sizeof(r32*));
        Assert(GridEntity->UnitSize);
        for(u32 i = 0; i < GridEntity->RowAmount; ++i)
        {
            GridEntity->UnitSize[i] = (r32*)malloc(sizeof(r32) * GridEntity->ColumnAmount);
            Assert(GridEntity->UnitSize[i]);
            for(u32 j = 0; j < GridEntity->ColumnAmount; ++j)
            {
                GridEntity->UnitSize[i][j] = 0;
            }
        }
        
        GameState->RotationVel         = 600.0f;
        GameState->StartAlphaPerSec    = 500.0f;
        GameState->FlippingAlphaPerSec = 1000.0f;
        GameState->GridScalePerSec     = ((RowAmount * ColumnAmount)) * (ActiveBlockSize/2);
        
        FigureEntityAlignHorizontally(FigureEntity, InActiveBlockSize);
        
        //
        // StickUnits initialization
        //
        GridEntity->StickUnits = (sticked_unit*)calloc(sizeof(sticked_unit), FigureEntity->FigureAmount);
        Assert(GridEntity->StickUnits);
        for (u32 i = 0; i < FigureEntity->FigureAmount; ++i)
        {
            GridEntity->StickUnits[i].Index     = -1;
            GridEntity->StickUnits[i].IsSticked = false;
        }
        
        //
        // MovingBlocks initialization
        //
        
        GridEntity->MovingBlocks = (moving_block*)malloc(sizeof(moving_block) * MovingBlocksAmountReserved);
        Assert(GridEntity->MovingBlocks);
        
        //GridEntityAddMovingBlock(GridEntity, 1, 3, false, true, ActiveBlockSize);
        //GridEntityAddMovingBlock(GridEntity, 3, 2, true,  false, ActiveBlockSize);
        
        //
        // GridEntity texture initialization
        //
        GridEntity->NormalSquareTexture     = GetTexture(Memory, "grid_cell.png", Buffer->Renderer);
        GridEntity->VerticalSquareTexture   = GetTexture(Memory, "o_s.png", Buffer->Renderer);
        GridEntity->HorizontlaSquareTexture = GetTexture(Memory, "o_m.png", Buffer->Renderer);
        
        
        // Level Editor initialization
        LevelEditorInit(GameState, Memory, Buffer);
        
        Memory->IsInitialized = true;
         printf("memory init!\n");
    }
    
    PreviousTimeTick = CurrentTimeTick;
    CurrentTimeTick  = SDL_GetTicks();
    
    r32 TimeElapsed = (CurrentTimeTick - PreviousTimeTick) / 1000.0f;
    
    GameUpdateEvent(Input, GameState, GameState->ActiveBlockSize, GameState->InActiveBlockSize, Buffer->Width, Buffer->Height);
    
    if(Input->WasPressed)
    {
        if(Input->Escape.IsDown)
        {
            ShouldQuit = true;
        }
    }
    
    //PrintArray2D(GridEntity->UnitField, GridEntity->RowAmount, GridEntity->ColumnAmount);
    
    game_rect ScreenArea = { 0, 0, Buffer->Width, Buffer->Height};
    DEBUGRenderQuadFill(Buffer, &ScreenArea, { 42, 6, 21 }, 255);
    
    LevelEntityUpdateAndRender(Buffer, GameState, TimeElapsed);
    LevelEditorUpdateAndRender(Memory->LevelEditor, GameState, Buffer, Input);
    
    //printf("TimeElapsed = %f\n", TimeElapsed);
    
    return(ShouldQuit);
}