#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "bsw.h"

struct tile *tiles = NULL;
int t_width, t_height, n_bombs, n_tiles_left;


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
    n_tiles_left = t_width * t_height - n_bombs;

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
    --n_tiles_left;
}

static void
expand_tile (struct tile *t, bool initial)
{
    if (!t || t->n_bombs != 0 || (!initial && t->status != TILE_NONE && t->status != TILE_MARKED))
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
        break;
    }
}
void
tile_draw (const struct tile *t, const SDL_Rect *rect)
{
    SDL_Rect srect;

    switch (t->status) {
    case TILE_NONE:
        srect.x = 0;
        srect.y = 16;
        break;
    case TILE_MARKED:
        srect.x = 16;
        srect.y = 16;
        break;
    case TILE_CLICKED:
        if (t->is_bomb) {
            srect.x = 32;
            srect.y = 16;
        } else {
            srect.x = t->n_bombs * 16;
            srect.y = 0;
        }
        break;
    }
    if (game_over && t->is_bomb && t->status != TILE_CLICKED) {
        srect.x = 48;
        srect.y = 16;
    }
    srect.w = 16;
    srect.h = 16;

    SDL_RenderCopy (renderer, sprite, &srect, rect);
}
