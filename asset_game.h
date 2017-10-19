/* assert_game.h --- 
 * 
 * Filename: assert_game.h
 * Author: Sierra
 * Created: Пн окт 16 16:29:29 2017 (+0300)
 * Last-Updated: Чт окт 19 21:15:39 2017 (+0400)
 *           By: Sierra
 */

#if !defined(ASSERT_GAME_H)

struct thread_data
{
		 SDL_Renderer *Renderer;
		 game_memory *Memory;

		 s64 ByteAmount;
		 bool IsInitialized;
};

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
};

struct asset_audio
{
		 asset_audio_header Header;
		 void *Data;
};

struct asset_bitmap
{
		 asset_bitmap_header Header;
		 void *Data;
};

struct asset_header
{
		 asset_header *Next;
		 asset_header *Prev;
		 
		 char AssetName[128];
		 u32 AssetSize;
		 asset_type AssetType;
		 union
		 {
					asset_audio  Audio;
		 			asset_bitmap Bitmap;
		 };
};

static game_texture* GetTexture(game_memory *Memory, char* FileName, SDL_Renderer*& Renderer);
static game_texture* GetSound(game_memory *Memory, char* FileName);
static game_texture* GetMusic(game_memory *Memory, char* FileName);

#define ASSERT_GAME_H
#endif
