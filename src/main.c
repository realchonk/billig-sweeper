#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "config.h"
#include "bsw.h"

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *sprite;
bool game_over;

void
reset_game (void)
{
    reset_tiles (n_bombs);
    game_over = false;
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

    menu_init ();

    // Game initialization.
    srand (time (NULL));
    if (!init_tiles (w, h, nb) || !init_SDL2 ())
        return 1;

    game_over = false;
    menu.shown = false;

    while (true) {
        SDL_Event e;

        if (SDL_WaitEvent (&e) && !handle_event (&e))
            break;

        render ();
    }

    quit_SDL2 ();
    free (tiles);
    return 0;
}
