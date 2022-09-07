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
#ifndef FILE_BSW_VIDEO_H
#define FILE_BSW_VIDEO_H
#include <SDL2/SDL.h>
#include <stdbool.h>

extern float t_offX, t_offY, t_size;
extern int w_width, w_height;
extern bool shift_pressed;

extern SDL_Window *window;
extern SDL_Renderer *renderer;
extern SDL_Texture *sprite;

bool video_init (void);
void video_quit (void);
void video_post_init (void);

void render (void);
bool handle_event (const SDL_Event *);

#endif // FILE_BSW_VIDEO_H
