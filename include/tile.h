#ifndef FILE_BSW_TILE_H
#define FILE_BSW_TILE_H
#include <SDL2/SDL_events.h>
#include <stdbool.h>

enum tile_status {
    TILE_NONE,                              // The default state of a tile.
    TILE_MARKED,                            // Clicked w/ right-click.
    TILE_CLICKED,                           // Clicked w/ left-click.
};

struct tile {
    enum tile_status status;                // Status of this tile.
    int x, y;                               // Position of this tile.
    unsigned n_bombs;                       // Number of bombs in the area.
    bool is_bomb;                           // Is this tile a bomb?
};

extern struct tile *tiles;
extern int t_width, t_height;
extern int n_bombs, n_tiles_left;

struct tile *get_tile (int x, int y);
bool tile_is_bomb (int x, int y);
void reset_tiles (int nb);
bool init_tiles (int w, int h, int nb);
void tile_handle_event (struct tile *, const SDL_Event *);
void tile_draw (const struct tile *, const SDL_Rect *);

#endif // FILE_BSW_TILE_H
