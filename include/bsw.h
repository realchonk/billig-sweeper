#ifndef FILE_BSW_H
#define FILE_BSW_H
#include <SDL2/SDL.h>
#include <stdbool.h>
#include "menu.h"
#include "tile.h"
#include "util.h"

#define TITLE "Billig Sweeper"
#define GITHUB_URL "https://github.com/riscygeek/billig-sweeper"
#define arraylen(a) (sizeof (a) / sizeof (*(a)))

extern SDL_Window *window;
extern SDL_Renderer *renderer;
extern SDL_Texture *sprite;
extern bool game_over;

bool init_SDL2 (void);
void quit_SDL2 (void);
void render (void);
void reset_game (void);
bool handle_event (const SDL_Event *);

#endif // FILE_BSW_H
