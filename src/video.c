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
#include "config.h"
#include "bsw.h"

static float tiles_scale = 1.5f;
static int tiles_offX = 5;
static int tiles_offY = 5;
static bool panning = false;
static int mouseX, mouseY;
bool shift_pressed = false;

bool
init_SDL2 ()
{
    SDL_RendererInfo renderInfo;
    SDL_Surface *surface;
    char *path_surface;

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




static void
draw_text (int idx, int ww, int wh)
{
    SDL_Rect srect, drect;

    srect.x = 0;
    srect.y = idx * 32;
    srect.w = 160;
    srect.h = 32;

    drect.w = ww / 2;
    drect.h = wh / 5;
    drect.x = (ww - drect.w) / 2;
    drect.y = (wh - drect.h) / 2;

    SDL_RenderCopy (renderer, sprite, &srect, &drect);
}

/*
 * Calculate the render dimensions:
 *   ts - tile size
 *   toffX - X render offset for tile map
 *   toffY - Y render offset for tile map
 *   ww - window width
 *   wh - window height
 */
static void
calc_tdims (int *ts, int *toffX, int *toffY, int ww, int wh)
{
    *ts = ww / t_width * tiles_scale;
    *toffX = tiles_offX + (ww - (*ts * t_width)) / 2;
    *toffY = tiles_offY + (wh - (*ts * t_height)) / 2;
}

void
render_tiles (int ww, int wh)
{
    int ts, toffX, toffY;
    calc_tdims (&ts, &toffX, &toffY, ww, wh);
    for (int y = 0; y < t_height; ++y) {
        for (int x = 0; x < t_width; ++x) {
            struct tile *t;
            SDL_Rect rect;

            t = get_tile (x, y);
            assert (t != NULL);

            rect.x = toffX + (x * ts);
            rect.y = toffY + (y * ts);
            rect.w = ts;
            rect.h = ts;

            tile_draw (t, &rect);
        }
    }
}

void
render (void)
{
    int ww, wh;
    SDL_GetWindowSize (window, &ww, &wh);
    // Clear the background.
    SDL_SetRenderDrawColor (renderer, default_color.r, default_color.g, default_color.b, 255);
    SDL_RenderClear (renderer);

    render_tiles (ww, wh);

    if (game_over)
        draw_text (all_selected () ? 1 : 2, ww, wh);

    if (menu.shown)
        menu_draw ();

    SDL_RenderPresent (renderer);
}

bool
handle_event (const SDL_Event *e)
{
    int ts, toffX, toffY, ww, wh;
    struct tile *t;

    SDL_GetWindowSize (window, &ww, &wh);
    calc_tdims (&ts, &toffX, &toffY, ww, wh);

    switch (e->type) {
    case SDL_QUIT:
        return false;
    case SDL_KEYDOWN:
        switch (e->key.keysym.sym) {
        case SDLK_LSHIFT:
            shift_pressed = true;
            break;
        default:
            if (menu.shown && !menu_handle_event (e))
                return false;
            break;
        }
        break;
    case SDL_KEYUP:
        switch (e->key.keysym.sym) {
        case SDLK_F1:
            open_url (GITHUB_URL);
            break;
        case SDLK_r:
            reset_game ();
            render ();
            break;
        case SDLK_ESCAPE:
        case SDLK_m:
            menu.shown = !menu.shown;
            render ();
            break;
        case SDLK_q:
            return false;
        case SDLK_LSHIFT:
            shift_pressed = false;
            break;
        default:
            if (menu.shown && !menu_handle_event (e))
                return false;
            break;
        }
        break;
    case SDL_MOUSEBUTTONUP: {
        if (panning) {
            panning = false;
            break;
        }
        if (menu.shown) {
            if (!menu_handle_event (e))
                return false;
            break;
        }

        if (game_over) {
            reset_game ();
            render ();
            break;
        }


        const int tx = (e->button.x - toffX) / ts;
        const int ty = (e->button.y - toffY) / ts;
        if (!generated)
            generate_tiles (tx, ty);
        t = get_tile (tx, ty);
        if (t) {
            tile_handle_event (t, e);
            break;
        }
        break;
    }
    case SDL_MOUSEMOTION: {
        mouseX = e->motion.x;
        mouseY = e->motion.y;
        if (e->motion.state == 1) {
            panning = true;
            tiles_offX += e->motion.xrel;
            tiles_offY += e->motion.yrel;
            render ();
        }
        break;
    }
    case SDL_MOUSEWHEEL: {
        const float preX = (float)(mouseX - toffX) / ts;
        const float preY = (float)(mouseY - toffY) / ts;

        // Zoom in/out with the scroll wheel.
        tiles_scale *= 1.0f + e->wheel.preciseY * 0.1f;
        tiles_scale = my_clamp (tiles_scale, 0.25f, 8.0f);

        calc_tdims (&ts, &toffX, &toffY, ww, wh);

        const float afterX = preX * ts + toffX;
        const float afterY = preY * ts + toffY;

        tiles_offX += mouseX - afterX;
        tiles_offY += mouseY - afterY;

        render ();
        break;
    }
    case SDL_WINDOWEVENT:
        switch (e->window.event) {
        case SDL_WINDOWEVENT_RESIZED:
        case SDL_WINDOWEVENT_MAXIMIZED:
        case SDL_WINDOWEVENT_SHOWN:
            menu_update (ww, wh);
            render ();
            break;
        }
        break;
    }

    return true;
}

