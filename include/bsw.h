/*
 * Copyright (C) 2022 Benjamin Stürz
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef FILE_BSW_H
#define FILE_BSW_H
#include <SDL2/SDL.h>
#include <stdbool.h>
#include "menu.h"
#include "tile.h"
#include "util.h"

#define TITLE "Billig Sweeper"
#define GITHUB_URL "https://github.com/riscygeek/billig-sweeper"
#define arraylen(a) (sizeof (a) / sizeof (*(a)))

extern SDL_Window *window;
extern SDL_Renderer *renderer;
extern SDL_Texture *sprite;
extern bool game_over;

bool init_SDL2 (void);
void quit_SDL2 (void);
void render (void);
void reset_game (void);
bool handle_event (const SDL_Event *);

#endif // FILE_BSW_H
