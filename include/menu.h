#ifndef FILE_BSW_MENU_H
#define FILE_BSW_MENU_H
#include <SDL2/SDL.h>
#include <stdbool.h>

struct menu;

enum button_id {
    BTN_QUIT,
    BTN_RESTART,
    BTN_ADD_BOMB,
    BTN_SUB_BOMB,
    BTN_ADD_HORIZ,
    BTN_SUB_HORIZ,
    BTN_ADD_VERT,
    BTN_SUB_VERT,
    LBL_BOMB,
    LBL_HORIZ,
    LBL_VERT,

    N_BUTTONS,
};

struct menu_button {
    enum button_id id;
    SDL_Rect trect;     // Texture rectangle.
    SDL_Rect mrect;     // Position relative to menu.
    SDL_Rect wrect;     // Position relative to window.
    bool (*on_click)(struct menu_button *);
    void (*on_move)(struct menu_button *);
};

struct menu {
    struct menu_button buttons[N_BUTTONS];
    SDL_Rect rect;
    bool shown;
};

extern struct menu menu;

void menu_init (void);
void menu_draw (void);
void menu_update (int ww, int wh);
bool menu_handle_event (const SDL_Event *);

#endif // FILE_BSW_MENU_H
