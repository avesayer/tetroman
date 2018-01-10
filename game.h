/* game.h --- 
 * 
 * Filename: game.h
 * Author: Sierra
 * Created: Пн окт  9 14:15:31 2017 (+0300)
 * Last-Updated: Пт окт 27 10:34:58 2017 (+0300)
 *           By: Sierra
 */

#if !defined(GAME_H)

void LogErrorLine(const char* Message, int Line)
{
    fprintf(stderr, "Assert fail in %s: %d\n",Message, Line);
}

#define Assert(Expression) if(!(Expression)) { LogErrorLine( __FILE__, __LINE__); *(int *)0 = 0;  }

struct game_offscreen_buffer
{
    int Width;
    int Height;
    SDL_Texture *Memory;
    SDL_Renderer *Renderer;
};

struct game_button_state
{
    bool IsDown;
    bool WasDown;
};

struct game_input
{
    bool WasPressed;
    bool MouseMotion;
    union
    {
        game_button_state Buttons[8];
        struct
        
        {
            game_button_state Up;
            game_button_state Down;
            game_button_state Left;
            game_button_state Right;
            game_button_state LeftClick;
            game_button_state RightClick;
            game_button_state Escape;
            game_button_state BackQuote;
            game_button_state Q_Button;
            game_button_state E_Button;
        };
    };
    
    s32 MouseX, MouseY;
    s32 MouseRelX, MouseRelY;
};

struct game_memory
{
    level_entity LevelEntity;
    level_editor *LevelEditor;
    
    menu_entity *MenuEntity;
    
    void *Assets;
    u32 AssetsSpaceAmount;
    
    level_memory* LevelMemory;
    u32 LevelMemoryAmount;
    u32 LevelMemoryReserved;
    
    game_music *Music;
    game_font *LevelNumberFont;
    
    bool IsInitialized;
    bool AssetsInitialized;
    bool LevelMemoryInitialized;
    bool ToggleMenu;
};

static bool GameUpdateAndRender(game_memory *Memory, game_input *Input,
                                game_offscreen_buffer *Buffer);

#define GAME_H
#endif
