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
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "bsw.h"

struct tile *tiles = NULL;
int t_width, t_height, n_bombs, n_selected;


struct tile *
get_tile (int x, int y)
{
    return (x >= 0 && x < t_width && y >= 0 && y < t_height)
           ? &tiles [y * t_width + x] : NULL;
}

bool
tile_is_bomb (int x, int y)
{
    const struct tile *t;
    return (t = get_tile (x, y)) != NULL && t->is_bomb;
}

void
reset_tiles (int nb)
{
    memset (tiles, 0, sizeof (struct tile) * t_width * t_height);

    n_bombs = nb;
    n_selected = 0;

    // Create bombs.
    while (nb > 0) {
        struct tile *t;
        int x, y;

        x = rrand (0, t_width - 1);
        y = rrand (0, t_height - 1);
        t = get_tile (x, y);
        assert (t != NULL);

        if (t->is_bomb)
            continue;

        t->is_bomb = true;
        --nb;
    }

    // Count bombs and initialize tile positions.
    for (int y = 0; y < t_height; ++y) {
        for (int x = 0; x < t_width; ++x) {
            struct tile *t;

            t = get_tile (x, y);
            assert (t != NULL);

            t->x = x;
            t->y = y;

            t->n_bombs += tile_is_bomb (x - 1, y - 1);
            t->n_bombs += tile_is_bomb (x    , y - 1);
            t->n_bombs += tile_is_bomb (x + 1, y - 1);
            t->n_bombs += tile_is_bomb (x - 1, y    );
            t->n_bombs += tile_is_bomb (x + 1, y    );
            t->n_bombs += tile_is_bomb (x - 1, y + 1);
            t->n_bombs += tile_is_bomb (x    , y + 1);
            t->n_bombs += tile_is_bomb (x + 1, y + 1);
        }
    }
}

bool
init_tiles (int w, int h, int nb)
{
    assert (w > 0 && h > 0 && nb > 0);
    free (tiles);
    tiles = malloc (w * h * sizeof (struct tile));
    if (!tiles) {
        perror ("malloc()");
        return false;
    }

    t_width = w;
    t_height = h;
    reset_tiles (nb);
    return true;
}

static void
select_tile (struct tile *t)
{
    if (t->status == TILE_CLICKED)
        return;
    t->status = TILE_CLICKED;
    if (!t->is_bomb)
        ++n_selected;
}

static void
expand_tile (struct tile *t, bool initial)
{
    if (!t || t->is_bomb || (!initial && t->status != TILE_NONE && t->status != TILE_MARKED))
        return;
    select_tile (t);
    expand_tile (get_tile (t->x - 1, t->y - 1), false);
    expand_tile (get_tile (t->x    , t->y - 1), false);
    expand_tile (get_tile (t->x + 1, t->y - 1), false);
    expand_tile (get_tile (t->x - 1, t->y    ), false);
    expand_tile (get_tile (t->x + 1, t->y    ), false);
    expand_tile (get_tile (t->x - 1, t->y + 1), false);
    expand_tile (get_tile (t->x    , t->y + 1), false);
    expand_tile (get_tile (t->x + 1, t->y + 1), false);
}

void
tile_handle_event (struct tile *t, const SDL_Event *e)
{
    switch (e->button.button) {
    case SDL_BUTTON_LEFT:
        select_tile (t);
        if (t->is_bomb) {
            game_over = true;
        } else {
            expand_tile (t, true);
        }
        render ();
        break;
    case SDL_BUTTON_RIGHT:
        switch (t->status) {
        case TILE_NONE:
            t->status = TILE_MARKED;
            break;
        case TILE_MARKED:
            t->status = TILE_NONE;
            break;
        case TILE_CLICKED:
            break;
        }
        render ();
        break;
    }

    if (all_selected ()) {
        game_over = true;
        render ();
    }
}
void
tile_draw (const struct tile *t, const SDL_Rect *rect)
{
    SDL_Rect srect, bgrect;

    srect.w = 16;
    srect.h = 16;

    bgrect.x = 0;
    bgrect.y = 16;
    bgrect.w = 16;
    bgrect.h = 16;

    switch (t->status) {
    case TILE_NONE:
        srect.x = 0;
        srect.y = 16;
        break;
    case TILE_MARKED:
        srect.x = 64;
        srect.y = 16;
        break;
    case TILE_CLICKED:
        if (t->is_bomb) {
            srect.x = 32;
            srect.y = 16;
        } else {
            bgrect.x = 16;

            srect.x = t->n_bombs * 16;
            srect.y = 0;
        }
        break;
    }

    // Render background tile.
    SDL_RenderCopy (renderer, sprite, &bgrect, rect);

    if (game_over && t->is_bomb && t->status != TILE_CLICKED) {
        srect.x = 48;
        srect.y = 16;
    }

    // Render actual tile.
    SDL_RenderCopy (renderer, sprite, &srect, rect);
}
