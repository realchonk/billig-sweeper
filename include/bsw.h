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
#include <stdnoreturn.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <time.h>
#include "menu.h"
#include "tile.h"
#include "util.h"

#define TITLE "Billig Sweeper"
#define GITHUB_URL "https://github.com/riscygeek/billig-sweeper"

extern SDL_Window *window;
extern SDL_Renderer *renderer;
extern SDL_Texture *sprite;
extern bool game_over, shift_pressed;
extern time_t start_time, end_time;
extern bool first_launch;

extern int default_n_mines;
extern int default_width;
extern int default_height;
extern SDL_Color default_color;
extern int default_presets[3][3]; // [3; [width, height, n_mines]]

bool init_SDL2 (void);
void quit_SDL2 (void);
void render (void);
void reset_game (void);
bool handle_event (const SDL_Event *);
void post_init_video (void);

void load_settings (void);
void save_settings (void);

noreturn void relaunch (void);

#endif // FILE_BSW_H
