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
