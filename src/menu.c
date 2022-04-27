#include "bsw.h"

struct menu menu;

static bool
btn_quit_on_click (struct menu_button *btn)
{
    (void)btn;
    return false;
}
static void
btn_quit_on_move (struct menu_button *btn)
{
    const float aspect = (float)btn->trect.w / (float)btn->trect.h;
    btn->mrect.h = menu.rect.h / 8;
    btn->mrect.w = (int)(btn->mrect.h * aspect);
    btn->mrect.x = menu.rect.w - btn->mrect.w - 10;
    btn->mrect.y = menu.rect.h - btn->mrect.h - 10;
}

static bool
btn_restart_on_click (struct menu_button *btn)
{
    (void)btn;
    reset_game ();
    menu.shown = false;
    return true;
}
static void
btn_restart_on_move (struct menu_button *btn)
{
    const float aspect = (float)btn->trect.w / (float)btn->trect.h;
    btn->mrect.h = menu.rect.h / 8;
    btn->mrect.w = (int)(btn->mrect.h * aspect);
    btn->mrect.x = 10;
    btn->mrect.y = menu.rect.h - btn->mrect.h - 10;
}

static void
draw_button (const struct menu_button *btn)
{
    SDL_RenderCopy (renderer, sprite, &btn->trect, &btn->wrect);
}

void
menu_init (void)
{
    // Init quit button
    menu.btn_quit.trect.x = 73;
    menu.btn_quit.trect.y = 96;
    menu.btn_quit.trect.w = 47;
    menu.btn_quit.trect.h = 32;
    menu.btn_quit.on_click = &btn_quit_on_click;
    menu.btn_quit.on_move = &btn_quit_on_move;

    // Init restart button
    menu.btn_restart.trect.x = 0;
    menu.btn_restart.trect.y = 96;
    menu.btn_restart.trect.w = 73;
    menu.btn_restart.trect.h = 32;
    menu.btn_restart.on_click = &btn_restart_on_click;
    menu.btn_restart.on_move = &btn_restart_on_move;

    menu.shown = false;
}

static void
button_update (struct menu_button *btn)
{
    btn->on_move (btn);
    btn->wrect.x = btn->mrect.x + menu.rect.x;
    btn->wrect.y = btn->mrect.y + menu.rect.y;
    btn->wrect.w = btn->mrect.w;
    btn->wrect.h = btn->mrect.h;
}

void
menu_update (int ww, int wh)
{
    menu.rect.w = ww * 3 / 4;
    menu.rect.h = wh * 3 / 4;
    menu.rect.x = (ww - menu.rect.w) / 2;
    menu.rect.y = (wh - menu.rect.h) / 2;

    button_update (&menu.btn_quit);
    button_update (&menu.btn_restart);
}

void
menu_draw (void)
{
    SDL_SetRenderDrawColor (renderer, 192, 192, 192, 255);
    SDL_RenderFillRect (renderer, &menu.rect);
    SDL_SetRenderDrawColor (renderer, 128, 128, 128, 255);
    SDL_RenderDrawRect (renderer, &menu.rect);

    draw_button (&menu.btn_quit);
    draw_button (&menu.btn_restart);
}
bool
menu_handle_event (const SDL_Event *e)
{
    switch (e->type) {
    case SDL_MOUSEBUTTONDOWN: {
        struct menu_button *buttons[2] = { &menu.btn_quit, &menu.btn_restart };
        SDL_Point p;
        p.x = e->button.x;
        p.y = e->button.y;
        for (size_t i = 0; i < arraylen(buttons); ++i) {
            struct menu_button *btn = buttons[i];
            if (SDL_PointInRect (&p, &btn->wrect)) {
                return btn->on_click (btn);
            }
        }
        break;
    }
    }

    return true;
}
