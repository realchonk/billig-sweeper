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
#include "dialog.h"
#include "bsw.h"

bool dialog_is_open = false;
static SDL_Rect rect;

void
dialog_init (void)
{
    if (first_launch)
        dialog_is_open = true;
}
void
dialog_draw (void)
{
    SDL_SetRenderDrawColor (renderer, 192, 192, 192, 255);
    SDL_RenderFillRect (renderer, &rect);

    SDL_SetRenderDrawColor (renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect (renderer, &rect);

    const SDL_Rect srect = { 0, 256, 160, 180 };
    SDL_RenderCopy (renderer, sprite, &srect, &rect);
}
void
dialog_update (int ww, int wh)
{
    rect.w = ww * 3 / 4;
    rect.h = wh * 3 / 4;
    rect.x = (ww - rect.w) / 2;
    rect.y = (wh - rect.h) / 2;
}
bool
dialog_click (SDL_Point p, int button)
{
    switch (button) {
    case SDL_BUTTON_LEFT:
        dialog_is_open = false;
        render ();
        break;
    case SDL_BUTTON_RIGHT:
        open_url (GITHUB_URL);
        break;
    }
    return true;
}
