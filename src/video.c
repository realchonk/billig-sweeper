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
#include <SDL2/SDL_image.h>
#include <assert.h>
#include "dialog.h"
#include "config.h"
#include "bsw.h"

static int t_size;
static float t_offX, t_offY;
static int w_width, w_height;
bool shift_pressed = false;

bool
init_SDL2 ()
{
    SDL_RendererInfo renderInfo;
    SDL_Surface *surface;
    char *path_surface;

    // Prefer Wayland by default, if SDL2 >= 2.0.22
#if SDL_MAJOR_VERSION >= 2 && (SDL_MINOR_VERSION >= 23 || (SDL_MINOR_VERSION == 0 && SDL_PATCHLEVEL == 22))
    SDL_SetHint (SDL_HINT_VIDEODRIVER, "wayland,x11");
#endif


    // Initialize SDL2 & SDL2_image.
    if (SDL_Init (SDL_INIT_VIDEO) != 0) {
        printf ("Failed to initialize SDL2: %s\n", SDL_GetError ());
        return false;
    }
    if (IMG_Init (IMG_INIT_PNG) != IMG_INIT_PNG) {
        printf ("Failed to initialize SDL2_image: %s\n", IMG_GetError ());
        SDL_Quit ();
        return false;
    }

    // Create a resizable window.
    window = SDL_CreateWindow (TITLE,
                               SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                               800, 600,
                               SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!window) {
        printf ("Failed to create window: %s\n", SDL_GetError ());
        IMG_Quit ();
        SDL_Quit ();
        return false;
    }

    // Create a hardware-accelerated renderer.
    renderer = SDL_CreateRenderer (window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
        renderer = SDL_CreateRenderer (window, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer) {
        printf ("Failed to create renderer: %s\n", SDL_GetError ());
        SDL_DestroyWindow (window);
        IMG_Quit ();
        SDL_Quit ();
        return false;
    }

    // Load the texture sprite/atlas.
    path_surface = relative_path (MSW_GRAPHICS_PNG);
    surface = IMG_Load (path_surface);
    if (!surface) {
        printf ("Failed to load '%s': %s\n", MSW_GRAPHICS_PNG, IMG_GetError ());
        free (path_surface);
        SDL_DestroyRenderer (renderer);
        SDL_DestroyWindow (window);
        IMG_Quit ();
        SDL_Quit ();
        return false;
    }
    free (path_surface);
    sprite = SDL_CreateTextureFromSurface (renderer, surface);
    if (!sprite) {
        printf ("Failed to create texture sprite: %s\n", SDL_GetError ());
        SDL_FreeSurface (surface);
        SDL_DestroyRenderer (renderer);
        SDL_DestroyWindow (window);
        IMG_Quit ();
        SDL_Quit ();
        return false;
    }
    SDL_FreeSurface (surface);

    // Set the window icon.
    path_surface = relative_path (MSW_ICON);
    surface = IMG_Load (path_surface);
    if (!surface) {
        printf ("Failed to load icon '%s': %s\n", path_surface, SDL_GetError ());
    }
    free (path_surface);
    if (surface) {
        SDL_SetWindowIcon (window, surface);
        SDL_FreeSurface (surface);
    }

    // Set the minimum window size to a reasonable value.
    SDL_SetWindowMinimumSize (window, 150, 100);

    // Print information about the.
    SDL_GetRendererInfo (renderer, &renderInfo);
    printf ("Renderer: %s\n", renderInfo.name);
    return true;
}

void
quit_SDL2 (void)
{
    SDL_DestroyTexture (sprite);
    SDL_DestroyRenderer (renderer);
    SDL_DestroyWindow (window);
    IMG_Quit ();
    SDL_Quit ();
}

void
post_init_video (void)
{
    SDL_GetWindowSize (window, &w_width, &w_height);

    t_size = my_min (w_width / t_width, w_height / t_height);
    t_offX = (float)(w_width - (t_width * t_size)) / t_size / 2;
    t_offY = (float)(w_height - (t_height * t_size)) / t_size / 2;
}

void
menu_draw_xint (unsigned value, unsigned len, int x0, int y0, int w, int h)
{
    const unsigned max_val = powui (10, len) - 1;
    SDL_Rect srect, rect;

    if (value > max_val)
        value = max_val;

    rect.x = x0;
    rect.y = y0;
    rect.w = len * w;
    rect.h = h;

    srect.y = 0;
    srect.w = 16;
    srect.h = 16;

    SDL_SetRenderDrawColor (renderer, 255, 255, 255, 255);
    SDL_RenderFillRect (renderer, &rect);
    SDL_SetRenderDrawColor (renderer, 128, 128, 128, 255);
    SDL_RenderDrawRect (renderer, &rect);

    for (unsigned i = len; i != 0; --i) {
        const unsigned digit = value % 10;
        const unsigned x = i-1;

        rect.x = x0 + (x * w);
        rect.y = y0;
        rect.w = w;
        rect.h = h;

        srect.x = 16 * digit;

        SDL_RenderCopy (renderer, sprite, &srect, &rect);

        value /= 10;
    }
}

static void
draw_text (int idx)
{
    SDL_Rect srect, drect;

    srect.x = 0;
    srect.y = idx * 64;
    srect.w = 160;
    srect.h = 64;

    drect.w = w_width / 2;
    drect.h = w_height * 2 / 5;
    drect.x = (w_width - drect.w) / 2;
    drect.y = (w_height - drect.h) / 2;

    SDL_RenderCopy (renderer, sprite, &srect, &drect);

    drect.y += 9 * drect.h / 16;
    drect.h = w_height / 8;

    menu_draw_int (end_time - start_time, 4, drect.x + drect.x * 2 / 17, drect.y, drect.w * 2 / 9, drect.h);
}

void
render_tiles (void)
{
    const int ox = t_offX * t_size, oy = t_offY * t_size;
    for (int y = 0; y < t_height; ++y) {
        for (int x = 0; x < t_width; ++x) {
            struct tile *t;
            SDL_Rect rect;

            t = get_tile (x, y);
            assert (t != NULL);

            rect.x = ox + (x * t_size);
            rect.y = oy + (y * t_size);
            rect.w = t_size;
            rect.h = t_size;

            tile_draw (t, &rect);
        }
    }
}

void
render (void)
{
    // Clear the background.
    SDL_SetRenderDrawColor (renderer, default_color.r, default_color.g, default_color.b, 255);
    SDL_RenderClear (renderer);

    render_tiles ();

    if (game_over)
        draw_text (all_selected () ? 1 : 2);

    if (menu.shown)
        menu_draw ();

    if (dialog_is_open)
        dialog_draw ();

    SDL_RenderPresent (renderer);
}

static void
click (SDL_Point p, int button)
{
    const int tx = (p.x - t_offX * t_size) / t_size;
    const int ty = (p.y - t_offY * t_size) / t_size;

    if (!generated)
        generate_tiles (tx, ty);

    struct tile *t = get_tile (tx, ty);

    if (t) {
        tile_click (t, button);
    }
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
        const int button = e->button.button;

        if (dialog_is_open) {
            dialog_click (p, button);
            break;
        }
        if (menu.shown) {
            if (!menu_click (p, button))
                return false;
            break;
        }

        if (game_over) {
            reset_game ();
            render ();
            break;
        }

        click (p, button);
        break;
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
