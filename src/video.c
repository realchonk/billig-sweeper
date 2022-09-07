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
#include "video.h"
#include "menu.h"
#include "tile.h"
#include "util.h"
#include "bsw.h"

SDL_Haptic *haptic;
float t_offX, t_offY, t_size;
int w_width, w_height;
bool shift_pressed = false;

bool
video_init ()
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

    haptic = SDL_HapticOpen (0);
    if (haptic) {
        SDL_HapticRumbleInit (haptic);
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
video_quit (void)
{
    SDL_DestroyTexture (sprite);
    SDL_DestroyRenderer (renderer);
    SDL_DestroyWindow (window);
    IMG_Quit ();
    SDL_Quit ();
}

void
video_post_init (void)
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
    const int ts = t_size;
    const int ox = t_offX * ts, oy = t_offY * ts;
    for (int y = 0; y < t_height; ++y) {
        for (int x = 0; x < t_width; ++x) {
            struct tile *t;
            SDL_Rect rect;

            t = get_tile (x, y);
            assert (t != NULL);

            rect.x = ox + (x * ts);
            rect.y = oy + (y * ts);
            rect.w = ts;
            rect.h = ts;

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

