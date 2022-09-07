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
#ifndef FILE_BSW_TILE_H
#define FILE_BSW_TILE_H
#include <SDL2/SDL_events.h>
#include <stdbool.h>

enum tile_status {
    TILE_NONE,                              // The default state of a tile.
    TILE_MARKED,                            // Clicked w/ right-click.
    TILE_MARKED2,                           // Clicked w/ right-click twice.
    TILE_CLICKED,                           // Clicked w/ left-click.
};

struct tile {
    enum tile_status status;                // Status of this tile.
    int x, y;                               // Position of this tile.
    unsigned n_bombs;                       // Number of bombs in the area.
    bool is_bomb;                           // Is this tile a bomb?
};

extern struct tile *tiles;
#define t_width default_width
#define t_height default_height
extern int n_bombs, n_selected;
extern bool generated;

struct tile *get_tile (int x, int y);
bool tile_is_bomb (int x, int y);
void generate_tiles (int x, int y);
void reset_tiles (void);
bool init_tiles (void);
void tile_click (struct tile *, int which);
void tile_draw (const struct tile *, const SDL_Rect *);

#define all_selected() (n_selected == (t_width * t_height - n_bombs))

#endif // FILE_BSW_TILE_H
