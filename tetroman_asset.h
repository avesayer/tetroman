/* ========================================= */
//     $File: tetroman_asset.cpp
//     $Date: June 16th 2017 10:08 am 
//     $Creator: Maksim Sokolov
//     $Revision: $
//     $Description: $
/* ========================================= */

#if !defined(ASSERT_GAME_H)

enum asset_type
{
    AssetType_None,
    AssetType_Bitmap,
    AssetType_Sound,
    AssetType_Music
};

struct asset_audio_header
{
    bool IsMusic;
};

struct asset_bitmap_header
{
    u32 Width;
    u32 Height;
    u32 Pitch;
    u32 Rmask;
    u32 Gmask;
    u32 Bmask;
    u32 Amask;
    u8 BytesPerPixel;
    u8 BitsPerPixel;
    SDL_Color Palette[256];
};

struct asset_audio
{
    asset_audio_header Header;
    u8 *Data;
};

struct asset_bitmap
{
    asset_bitmap_header Header;
    void *Data;
};

struct asset_header
{
    char AssetName[32];
    u32 AssetSize;
    asset_type AssetType;
    union
    {
        asset_audio  Audio;
        asset_bitmap Bitmap;
    };
};

struct binary_header
{
    u32 BitmapSizeInBytes;
    u32 AudioSizeInBytes;
};

static game_sound*   GetSound(game_memory *Memory, char* FileName);
static game_music*   GetMusic(game_memory *Memory, char* FileName);
static game_texture* GetTexture(game_memory *Memory, const char* FileName, SDL_Renderer* Renderer);

static void FreeSound(game_sound *&Sound)
{
    if(Sound)
    {
        Mix_FreeChunk(Sound);
    }
}

static void FreeMusic(game_music *&Music)
{
    if(Music)
    {
        Mix_FreeMusic(Music);
    }
}

static void FreeTexture(game_texture *&Texture)
{
    if(Texture)
    {
        SDL_DestroyTexture(Texture);
    }
}

struct figure_data
{
    r32 Angle;
    figure_flip Flip;
    figure_form Form;
    figure_type Type;
};

struct moving_block_data
{
    u32 RowNumber;
    u32 ColNumber;
    bool IsVertical;
    bool MoveSwitch;
};

struct playground_data
{
    b32 IsUnlocked;
    u32 LevelNumber;
    u32 RowAmount;
    u32 ColumnAmount;
    u32 MovingBlocksAmount;
    u32 FigureAmount;
    
    s32 UnitField[COLUMN_AMOUNT_MAXIMUM * ROW_AMOUNT_MAXIMUM];
    moving_block_data MovingBlocks[MOVING_BLOCKS_MAXIMUM];
    figure_data Figures[FIGURE_AMOUNT_MAXIMUM];
};

#define ASSERT_GAME_H
#endif
