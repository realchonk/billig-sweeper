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
#include <stdbool.h>
#include "dialog.h"
#include "video.h"
#include "tile.h"
#include "menu.h"
#include "util.h"
#include "bsw.h"

static bool
click (SDL_Point p, int button)
{
    if (dialog_is_open)
        return dialog_click (p, button);

    if (menu.shown)
        return menu_click (p, button);

    if (game_over) {
        reset_game ();
        render ();
        return true;
    }

    const int tx = (p.x - t_offX * t_size) / t_size;
    const int ty = (p.y - t_offY * t_size) / t_size;

    if (!generated)
        generate_tiles (tx, ty);

    struct tile *t = get_tile (tx, ty);

    if (t) {
        tile_click (t, button);
        return true;
    }

    return true;
}

static void
zoom (int ax, int ay, float factor)
{
    const float preX = (float)(ax - t_offX * t_size) / t_size;
    const float preY = (float)(ay - t_offY * t_size) / t_size;

    // Zoom in/out with the scroll wheel.
    const float mx = my_min (w_width / 5, w_height / 5);
    t_size = my_clamp (t_size * factor, 10.0f, mx);

    const float afterX = (float)(ax - t_offX * t_size) / t_size;
    const float afterY = (float)(ay - t_offY * t_size) / t_size;

    // Adjust the position of the tiles to have the same relative position.
    t_offX += afterX - preX;
    t_offY += afterY - preY;

    render ();
}

bool
handle_event (const SDL_Event *e)
{
    static int mouseX, mouseY;
    static bool space_pressed = false;

    switch (e->type) {
    case SDL_QUIT:
        return false;
    case SDL_KEYDOWN:
        switch (e->key.keysym.sym) {
        case SDLK_LSHIFT:
            shift_pressed = true;
            break;
        case SDLK_SPACE:
            space_pressed = true;
            break;
        }
        break;
    case SDL_KEYUP:
        switch (e->key.keysym.sym) {
        case SDLK_F1:
            dialog_is_open = !dialog_is_open;
            render ();
            break;
        case SDLK_r:
            if (e->key.keysym.mod & KMOD_CTRL)
                relaunch ();
            reset_game ();
            render ();
            break;
        case SDLK_ESCAPE:
            if (dialog_is_open) {
                dialog_is_open = false;
                render ();
                break;
            }
            SDL_FALLTHROUGH;
        case SDLK_m:
            menu.shown = !menu.shown;
            render ();
            break;
        case SDLK_q:
            return false;
        case SDLK_LSHIFT:
            shift_pressed = false;
            break;
        case SDLK_SPACE:
            space_pressed = false;
            break;
        }
        break;
    case SDL_MOUSEBUTTONUP: {
        const SDL_Point p = { e->button.x, e->button.y };

        return click (p, e->button.button);
    }
    case SDL_MOUSEMOTION: {
        const int dx = e->motion.xrel;
        const int dy = e->motion.yrel;
        mouseX = e->motion.x;
        mouseY = e->motion.y;

        // Panning
        if ((space_pressed || e->motion.state == SDL_BUTTON_MIDDLE) && !(game_over || menu.shown || dialog_is_open)) {
            t_offX += (float)dx / t_size;
            t_offY += (float)dy / t_size;

            // Limit the amount of panning.
            t_offX = my_clamp (t_offX, -t_width + 1, ((float)w_width / t_size) - 1);
            t_offY = my_clamp (t_offY, -t_height + 1, ((float)w_height / t_size) - 1);

            render ();
        }
        break;
    }
    case SDL_MOUSEWHEEL: {
        zoom (mouseX, mouseY, 1.0f + e->wheel.preciseY * 0.1f);
        break;
    }
    case SDL_WINDOWEVENT:
        switch (e->window.event) {
        case SDL_WINDOWEVENT_RESIZED:
        case SDL_WINDOWEVENT_MAXIMIZED:
        case SDL_WINDOWEVENT_SHOWN: {
            const float corner_x = (t_offX + t_width / 2) * t_size / w_width;
            const float corner_y = (t_offY + t_height / 2) * t_size / w_height;

            SDL_GetWindowSize (window, &w_width, &w_height);

            // Adjust the center of the playing field.
            t_offX = corner_x * w_width / t_size - t_width / 2;
            t_offY = corner_y * w_height / t_size - t_height / 2;

            menu_update (w_width, w_height);
            dialog_update (w_width, w_height);
            render ();
            break;
        }
        }
        break;
    }

    return true;
}
