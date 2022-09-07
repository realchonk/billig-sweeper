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
#ifndef FILE_BSW_MENU_H
#define FILE_BSW_MENU_H
#include <SDL2/SDL.h>
#include <stdbool.h>

struct menu;

enum button_id {
    BTN_QUIT,
    BTN_RESTART,
    BTN_ADD_BOMB,
    BTN_SUB_BOMB,
    BTN_ADD_HORIZ,
    BTN_SUB_HORIZ,
    BTN_ADD_VERT,
    BTN_SUB_VERT,
    BTN_MIDGET,
    BTN_HALF_MIDGET,
    BTN_NON_MIDGET,
    LBL_BOMB,
    LBL_HORIZ,
    LBL_VERT,

    N_BUTTONS,
};

struct menu_button {
    enum button_id id;
    SDL_Rect trect;     // Texture rectangle.
    SDL_Rect mrect;     // Position relative to menu.
    SDL_Rect wrect;     // Position relative to window.
    bool (*on_click)(struct menu_button *);
    void (*on_move)(struct menu_button *);
};

struct menu {
    struct menu_button buttons[N_BUTTONS];
    SDL_Rect rect;
    bool shown;
};

extern struct menu menu;

void menu_init (void);
void menu_draw (void);
void menu_update (int ww, int wh);
bool menu_click (SDL_Point p, int button);
void menu_draw_int (unsigned value, unsigned len, int x0, int y0, int w, int h);

#endif // FILE_BSW_MENU_H
