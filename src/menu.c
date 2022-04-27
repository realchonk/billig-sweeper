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
    menu.buttons[BTN_QUIT].trect.x = 73;
    menu.buttons[BTN_QUIT].trect.y = 96;
    menu.buttons[BTN_QUIT].trect.w = 47;
    menu.buttons[BTN_QUIT].trect.h = 32;
    menu.buttons[BTN_QUIT].on_click = &btn_quit_on_click;
    menu.buttons[BTN_QUIT].on_move = &btn_quit_on_move;

    // Init restart button
    menu.buttons[BTN_RESTART].trect.x = 0;
    menu.buttons[BTN_RESTART].trect.y = 96;
    menu.buttons[BTN_RESTART].trect.w = 73;
    menu.buttons[BTN_RESTART].trect.h = 32;
    menu.buttons[BTN_RESTART].on_click = &btn_restart_on_click;
    menu.buttons[BTN_RESTART].on_move = &btn_restart_on_move;

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

    button_update (&menu.buttons[BTN_QUIT]);
    button_update (&menu.buttons[BTN_RESTART]);
}

void
menu_draw (void)
{
    SDL_SetRenderDrawColor (renderer, 192, 192, 192, 255);
    SDL_RenderFillRect (renderer, &menu.rect);
    SDL_SetRenderDrawColor (renderer, 128, 128, 128, 255);
    SDL_RenderDrawRect (renderer, &menu.rect);

    draw_button (&menu.buttons[BTN_QUIT]);
    draw_button (&menu.buttons[BTN_RESTART]);
}
bool
menu_handle_event (const SDL_Event *e)
{
    switch (e->type) {
    case SDL_MOUSEBUTTONDOWN: {
        SDL_Point p;
        p.x = e->button.x;
        p.y = e->button.y;
        for (size_t i = 0; i < N_BUTTONS; ++i) {
            struct menu_button *btn = &menu.buttons[i];
            if (SDL_PointInRect (&p, &btn->wrect)) {
                return btn->on_click (btn);
            }
        }
        break;
    }
    }

    return true;
}
