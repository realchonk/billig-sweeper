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
#include <stdlib.h> // rand(), malloc(), abort()
#include <limits.h> // PATH_MAX
#include <unistd.h> // readlink()
#include <libgen.h> // dirname()
#include <stdio.h>  // perror()
#include "util.h"

int
rrand (int min_val, int max_val)
{
    return min_val + (((float)rand () / (float)RAND_MAX) * (max_val - min_val + 1));
}

char *
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

unsigned
powui (unsigned base, unsigned exponent)
{
    unsigned result = 1;

    while (exponent > 0) {
        if (exponent & 1)
            result *= base;
        base *= base;
        exponent >>= 1;
    }

    return result;
}

void
open_url (const char *url)
{
    char buffer[512];
    snprintf (buffer, sizeof buffer, "xdg-open '%s'", url);
    system (buffer);
}
