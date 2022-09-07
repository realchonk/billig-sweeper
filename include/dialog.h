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
#ifndef FILE_BSW_DIALOG_H
#define FILE_BSW_DIALOG_H
#include <SDL2/SDL.h>
#include <stdbool.h>

extern bool dialog_is_open;

void dialog_init (void);
void dialog_draw (void);
void dialog_update (int ww, int wh);
bool dialog_click (SDL_Point p, int button);

#endif // FILE_BSW_DIALOG_H
