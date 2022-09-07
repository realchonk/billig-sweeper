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

#define TOUCH_DELAY 500

#define stop_timer(tid)         \
do {                            \
    if (tid) {                  \
        SDL_RemoveTimer (tid);  \
        tid = 0;                \
    }                           \
} while (0)

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

    const int ts = t_size;
    const int tx = (p.x - t_offX * ts) / ts;
    const int ty = (p.y - t_offY * ts) / ts;

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
zoom (SDL_Point p, float factor)
{
    int ts = t_size;
    const float preX = (float)(p.x - t_offX * ts) / ts;
    const float preY = (float)(p.y - t_offY * ts) / ts;

    // Zoom in/out with the scroll wheel.
    const float mx = my_min (w_width / 5, w_height / 5);
    t_size = my_clamp (t_size * factor, 10.0f, mx);
    ts = (int)t_size;

    const float afterX = (float)(p.x - t_offX * ts) / ts;
    const float afterY = (float)(p.y - t_offY * ts) / ts;

    // Adjust the position of the tiles to have the same relative position.
    t_offX += afterX - preX;
    t_offY += afterY - preY;

    render ();
}

static void
pan (SDL_Point delta)
{
    if (menu.shown || dialog_is_open)
        return;

    const int ts = t_size;
    t_offX += (float)delta.x / ts;
    t_offY += (float)delta.y / ts;

    // Limit the amount of panning.
    t_offX = my_clamp (t_offX, -t_width + 1, ((float)w_width / ts) - 1);
    t_offY = my_clamp (t_offY, -t_height + 1, ((float)w_height / ts) - 1);

    render ();
}

static Uint32
cb_touch (Uint32 interval, void *arg)
{
    (void)interval;
    const SDL_Point *p = arg;

    // TODO: haptic feedback

    click (*p, SDL_BUTTON_RIGHT);
    return 0;
}

bool
handle_event (const SDL_Event *e)
{
    static int mouseX, mouseY, num_fingers = 0;
    static bool space_pressed = false;
    static SDL_TimerID touch_timerID = 0;
    static SDL_Point touch_pos;

    switch (e->type) {
    // Keyboard-related
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

    // Mouse-related
    case SDL_MOUSEBUTTONUP: {
        if (e->button.which == SDL_TOUCH_MOUSEID)
            break;
        const SDL_Point p = { e->button.x, e->button.y };
        return click (p, e->button.button);
    }
    case SDL_MOUSEMOTION:
        if (e->button.which == SDL_TOUCH_MOUSEID)
            break;
        mouseX = e->motion.x;
        mouseY = e->motion.y;

        // Panning
        if (space_pressed || e->motion.state == SDL_BUTTON_MIDDLE) {
            const SDL_Point delta = { e->motion.xrel, e->motion.yrel };
            pan (delta);
        }
        break;
    case SDL_MOUSEWHEEL: {
        if (e->button.which == SDL_TOUCH_MOUSEID)
            break;
        const SDL_Point p = { mouseX, mouseY };
        zoom (p, 1.0f + e->wheel.preciseY * 0.1f);
        break;
    }

    // Touchscreen-related
    case SDL_FINGERDOWN:
        ++num_fingers;
        if (touch_timerID) {
            stop_timer (touch_timerID);
        } else if (num_fingers == 1) {
            touch_pos.x = e->tfinger.x * w_width;
            touch_pos.y = e->tfinger.y * w_height;
            touch_timerID = SDL_AddTimer (TOUCH_DELAY, &cb_touch, &touch_pos);
        }
        break;
    case SDL_FINGERUP:
        --num_fingers;
        if (touch_timerID != 0 && num_fingers == 0) {
            const SDL_Point p = { e->tfinger.x * w_width, e->tfinger.y * w_height };
            click (p, SDL_BUTTON_LEFT);
        }
        stop_timer (touch_timerID);
        break;
    case SDL_FINGERMOTION:
        if (num_fingers == 1) {
            const float dx = e->tfinger.dx;
            const float dy = e->tfinger.dy;
            const float dist = sqrtf (dx*dx + dy*dy);
            if (dist < 0.003f)
                break;

            stop_timer (touch_timerID);
            const SDL_Point p = { dx * w_width, dy * w_height };
            pan (p);
        }
        break;
    case SDL_MULTIGESTURE:
        if (e->mgesture.numFingers == 2) {
            stop_timer (touch_timerID);
            const SDL_Point p = { e->mgesture.x * w_width, e->mgesture.y * w_height };
            zoom (p, 1.0f + e->mgesture.dDist * 5.0f);
        }
        break;

    // Other
    case SDL_QUIT:
        return false;
    case SDL_WINDOWEVENT:
        switch (e->window.event) {
        case SDL_WINDOWEVENT_RESIZED:
        case SDL_WINDOWEVENT_MAXIMIZED:
        case SDL_WINDOWEVENT_SHOWN: {
            const int ts = t_size;
            const float corner_x = (t_offX + t_width / 2) * ts / w_width;
            const float corner_y = (t_offY + t_height / 2) * ts / w_height;

            SDL_GetWindowSize (window, &w_width, &w_height);

            // Adjust the center of the playing field.
            t_offX = corner_x * w_width / ts - t_width / 2;
            t_offY = corner_y * w_height / ts - t_height / 2;

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
