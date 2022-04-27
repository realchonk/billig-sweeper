#include <SDL2/SDL_image.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <libgen.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "config.h"

#define TITLE "Billig-Sweeper"

enum tile_status {
    TILE_NONE,                  // The default state of a tile.
    TILE_MARKED,                // Clicked w/ right-click.
    TILE_CLICKED,               // Clicked w/ left-click.
};

struct tile {
    enum tile_status status;    // Status of this tile.
    int x, y;                   // Position of this tile.
    unsigned n_bombs;           // Number of bombs in the area.
    bool is_bomb;               // Is this tile a bomb?
};

static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *sprite;
static struct tile *tiles;
static int t_width, t_height, n_bombs, n_tiles_left;
static bool game_over, show_menu;


/*
 * Get a tile at position (x,y) or NULL.
 */
static struct tile *
get_tile (int x, int y)
{
    return (x >= 0 && y >= 0 && x < t_width && y < t_height) ? &tiles [y * t_width + x] : NULL;
}

/*
 * Check if a tile at position (x,y) is a bomb.
 */
static bool
tile_is_bomb (int x, int y)
{
    const struct tile *t = get_tile (x, y);
    return t != NULL && t->is_bomb;
}

/*
 * Generate a random number between `min_val` and `max_val`.
 */
static int
rrand (int min_val, int max_val)
{
    return min_val + (((float)rand () / (float)RAND_MAX) * (max_val - min_val + 1));
}

static void
reset_map (int nb)
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

/*
 * Initialize the map of tiles.
 */
static bool
init_tiles (int w, int h, int nb)
{
    assert (w > 0 && h > 0);

    tiles = malloc (w * h * sizeof (struct tile));
    if (!tiles) {
        perror ("calloc()");
        return false;
    }

    t_width = w;
    t_height = h;
    
    reset_map (nb);
    
    return true;
}

/*
 * Make a relative path.
 */
static char *
relative_path (const char *p)
{
    char self [PATH_MAX];
    if (readlink ("/proc/self/exe", self, sizeof self) < 0) {
        perror ("readlink('/proc/self/exe')");
        abort ();
    }
    dirname (self);
    
    char *path = malloc (PATH_MAX);
    if (!path) {
        perror ("malloc()");
        abort ();
    }
    
    if (snprintf (path, PATH_MAX, "%s/../%s", self, p) >= PATH_MAX) {
        abort ();
    }
    return path;
}

/*
 * Initialize the SDL2 rendering backend.
 */
static bool
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
    SDL_SetWindowMinimumSize (window, 200, 200);

    // Print information about the.
    SDL_GetRendererInfo (renderer, &renderInfo);
    printf ("Renderer: %s\n", renderInfo.name);

    return true;
}

/*
 * Destroy all SDL2-related resources.
 */
static void
quit_SDL2 ()
{
    SDL_DestroyTexture (sprite);
    SDL_DestroyRenderer (renderer);
    SDL_DestroyWindow (window);
    IMG_Quit ();
    SDL_Quit ();
}

/*
 * Draw a tile `t` at `rect`.
 */
static void
draw_tile (const struct tile *t, const SDL_Rect *rect)
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

static void
draw_text (int idx)
{
    int ww, wh;
    SDL_Rect srect, drect;

    SDL_GetWindowSize (window, &ww, &wh);

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

static void
draw_menu (int ww, int wh)
{
    SDL_Rect rect, srect, drect;

    // Render the background.
    rect.w = ww * 3 / 4;
    rect.h = wh * 3 / 4;
    rect.x = (ww - rect.w) / 2;
    rect.y = (wh - rect.h) / 2;
    SDL_SetRenderDrawColor (renderer, 192, 192, 192, 255);
    SDL_RenderFillRect (renderer, &rect);
    SDL_SetRenderDrawColor (renderer, 128, 128, 128, 255);
    SDL_RenderDrawRect (renderer, &rect);

    // Render the restart button.
    srect.x = 0;
    srect.y = 96;
    srect.w = 73;
    srect.h = 32;
    drect.w = srect.w * 2;
    drect.h = srect.h * 2;
    drect.x = rect.x + rect.w - (drect.w) - 20;
    drect.y = rect.y + rect.h - (drect.h) - 20;
    SDL_RenderCopy (renderer, sprite, &srect, &drect);
}

static bool
menu_handle_event (const SDL_Event *e)
{
    switch (e->type) {
    case SDL_KEYDOWN:
        break;

    case SDL_MOUSEBUTTONDOWN:
        break;
    }
    return true;
}

static bool
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
            reset_map (n_bombs);
            game_over = false;            break;
        case SDLK_m:
            show_menu = !show_menu;            break;
        case SDLK_q:
            return false;
        default:
            if (show_menu && !menu_handle_event (e))
                return false;
            break;
        }
        break;
    case SDL_MOUSEBUTTONDOWN:
        if (game_over)
            break;

        if (show_menu) {
            if (!menu_handle_event (e))
                return false;
            break;
        }

        t = get_tile ((e->button.x - toffX) / tw,
                      (e->button.y - toffY) / th);
        if (!t)
            break;
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
        break;
    }
    return true;

}
static void
render ()
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

            draw_tile (t, &rect);
        }
    }

    if (show_menu)
        draw_menu (ww, wh);

    if (n_tiles_left == 0)
        game_over = true;

    if (game_over) {
        draw_text (n_tiles_left == 0 ? 1 : 2);
    }

    SDL_RenderPresent (renderer);
}
int
main (int argc, char *argv[])
{
    int option, w = 10, h = 10, nb = 10;

    while ((option = getopt (argc, argv, ":hVr:s:n:")) != -1) {
        char *endp;
        switch (option) {
        case 'h':
            puts (
                TITLE " v" MSW_VERSION "\n"
                "\n"
                "Usage: billig-sweeper [OPTION]...\n"
                "A minesweeper clone made in an afternoon.\n"
                "\n"
                "Options:\n"
                "  -h                    Show this help page.\n"
                "  -V                    Print the version.\n"
                "  -s <width>x<height>   Specify the map size. (default: 10x10)\n"
                "  -n <integer>          Specify how many bombs you want. (default: 10)\n"
                "\n"
                "Report bugs to <benni@stuerz.xyz>"
            );
            return 0;
        case 'V':
            printf ("%s v%s.", TITLE, MSW_VERSION);
            return 0;
        case 's':
            if (sscanf (optarg, "%dx%d", &w, &h) != 2) {
                printf ("Invalid size: %s\n", optarg);
                return 1;
            }
            break;
        case 'n':
            nb = (int)strtol (optarg, &endp, 10);
            if (*endp || nb < 1) {
                printf ("Invalid number of bombs: %s\n", optarg);
                return 1;
            }
            break;
        case '?':
            printf ("Invalid option '-%c'.\n", optopt);
            return 1;
        case ':':
            printf ("Expected argument for option '-%c'.\n", optopt);
            return 1;
        }
    }

    // Initialization.
    srand (time (NULL));
    if (!init_tiles (w, h, nb) || !init_SDL2 ())
        return 1;
    game_over = false;
    show_menu = false;
    while (true) {
        // Check for events.
        SDL_Event e;

        if (SDL_WaitEvent (&e) && !handle_event (&e))
            goto quit;

        render ();
    }

quit:
    quit_SDL2 ();
    free (tiles);
    return 0;
}
