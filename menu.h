// menu_game.h --- 
// 
// Filename: game.cpp
// Author: Sierra
// Created: Вт окт 10 10:32:14 2017 (+0300)
// Last-Updated: Пт окт 27 14:17:15 2017 (+0300)
//           By: Sierra
//

#if !defined(MENU_GAME_H)

static void MenuInit(menu_entity *MenuEntity, game_memory *Memory, game_offscreen_buffer *Buffer);

static void MenuUpdateAndRender(game_offscreen_buffer *Buffer, game_memory *Memory, game_input *Input);

#define MENU_GAME_H
#endif