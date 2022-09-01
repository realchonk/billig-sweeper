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
#ifndef FILE_BSW_UTIL_H
#define FILE_BSW_UTIL_H

#define arraylen(a) (sizeof (a) / sizeof (*(a)))
#define my_min(a,b) ((a) < (b) ? (a) : (b))
#define my_max(a,b) ((a) > (b) ? (a) : (b))
#define my_clamp(v,mn,mx) (my_min (mx, my_max (v, mn)))

// Generate a random number between `min_val` and `max_val`.
int rrand (int min_val, int max_val);

// Create a path relative to the executable.
char *relative_path (const char *path);

// pow() for unsigned integers
unsigned powui (unsigned base, unsigned exponent);

// Opens a URL in a browser.
void open_url (const char *url);

// Create directory and parents.
void mkdir_p (char *);

#endif // FILE_BSW_UTIL_H
