#include <SDL2/SDL_image.h>
#include <assert.h>
#include "config.h"
#include "bsw.h"

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
    SDL_SetWindowMinimumSize (window, 400, 200);

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
 *   tw - tile width
 *   th - tile height
 *   toffX - X render offset for tile map
 *   toffY - Y render offset for tile map
 */
static void
calc_tdims (int *tw, int *th, int *toffX, int *toffY, int ww, int wh)
{
    const int ctoffX = 5, ctoffY = 5;

    *tw = (ww - ctoffX * 2) / t_width;
    *th = (wh - ctoffY * 2) / t_height;
    *toffX = (ww - (*tw * t_width)) / 2;
    *toffY = (wh - (*th * t_height)) / 2;
}


void
render (void)
{
        int tw, th, toffX, toffY, ww, wh;
        SDL_GetWindowSize (window, &ww, &wh);
        calc_tdims (&tw, &th, &toffX, &toffY, ww, wh);
    // Clear the background.
    SDL_SetRenderDrawColor (renderer, 255, 0, 255, 255);
    SDL_RenderClear (renderer);

    // Render all tiles.
    for (int y = 0; y < t_height; ++y) {
        for (int x = 0; x < t_width; ++x) {
            struct tile *t;
            SDL_Rect rect;

            t = get_tile (x, y);
            assert (t != NULL);

            rect.x = toffX + (x * tw);
            rect.y = toffY + (y * th);
            rect.w = tw;
            rect.h = th;

            tile_draw (t, &rect);
        }
    }

    if (game_over)
        draw_text (n_tiles_left == 0 ? 1 : 2, ww, wh);

    if (menu.shown)
        menu_draw ();

    SDL_RenderPresent (renderer);
}

bool
handle_event (const SDL_Event *e)
{
    int tw, th, toffX, toffY, ww, wh;
    struct tile *t;

    SDL_GetWindowSize (window, &ww, &wh);
    calc_tdims (&tw, &th, &toffX, &toffY, ww, wh);

    switch (e->type) {
    case SDL_QUIT:
        return false;
    case SDL_KEYUP:
        switch (e->key.keysym.sym) {
        case SDLK_r:
            reset_game ();
            break;
        case SDLK_m:
            menu.shown = !menu.shown;
            break;
        case SDLK_q:
            return false;
        default:
            if (menu.shown && !menu_handle_event (e))
                return false;
            break;
        }
        break;
    case SDL_MOUSEBUTTONDOWN:
        if (menu.shown) {
            if (!menu_handle_event (e))
                return false;
            break;
        }

        if (game_over)
            break;


        t = get_tile ((e->button.x - toffX) / tw,
                      (e->button.y - toffY) / th);
        if (t) {
            tile_handle_event (t, e);
            break;
        }
        break;
    }

    if (menu.shown)
        menu_update (ww, wh);

    if (n_tiles_left == 0)
        game_over = true;

    return true;
}

