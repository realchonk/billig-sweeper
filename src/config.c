#include <libgen.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "toml.h"
#include "tile.h"
#include "bsw.h"

int default_n_mines     = 10;
int default_width       = 10;
int default_height      = 10;
SDL_Color default_color = { .r = 255, .g = 0, .b = 255, .a = 255, };

#define read_val_int(x)                                                 \
x = toml_int_in (game, #x);                                             \
if (x.ok) {                                                             \
    default_##x = (int)x.u.i;                                           \
} else {                                                                \
    fprintf (stderr, "Missing field 'Games.%s' in '%s'", #x, filename); \
}

char *
make_filename (void)
{
    char *xdg_config = getenv ("XDG_CONFIG_DIR");
    if (xdg_config) {
        const char suffix[] = "billig-sweeper/config.toml";
        const size_t len = strlen (xdg_config) + sizeof suffix + 3;
        char *filename = malloc (len);
        snprintf (filename, len, "%s/%s", xdg_config, suffix);
        return filename;
    }

    char *home = getenv ("HOME");
    if (home) {
        const char suffix[] = ".config/billig-sweeper/config.toml";
        const size_t len = strlen (home) + sizeof suffix + 3;
        char *filename = malloc (len);
        snprintf (filename, len, "%s/%s", home, suffix);
        return filename;
    }

    fputs ("Failed to find valid location for config. Neither $XDG_CONFIG_DIR, nor $HOME is set.\n", stderr);
    return NULL;
}

void
load_settings (void)
{
    char *filename = make_filename ();
    FILE *file = fopen (filename, "r");
    if (!file) {
        fprintf (stderr, "Failed to open '%s': %s\n", filename, strerror (errno));
        goto end;
    }

    char errbuf[200];
    toml_table_t *conf = toml_parse_file (file, errbuf, sizeof errbuf);
    fclose (file);

    if (!conf) {
        fprintf (stderr, "Failed to parse '%s': %s\n", filename, errbuf);
        goto end;
    }

    toml_table_t *game = toml_table_in (conf, "Game");
    if (!game) {
        fprintf (stderr, "Missing section 'Game' in '%s'\n", filename);
        goto end;
    }

    toml_datum_t n_mines, width, height;
    read_val_int(n_mines);
    read_val_int(width);
    read_val_int(height);

    toml_array_t *color = toml_array_in (game, "color");
    if (color != NULL) {
        int rgb[3];
        for (int i = 0; i < 3; ++i) {
            toml_datum_t c = toml_int_at (color, i);
            if (c.ok) {
                rgb[i] = c.u.i;
            } else {
                goto invalid_color;
            }
        }
        default_color.r = rgb[0];
        default_color.g = rgb[1];
        default_color.b = rgb[2];
        default_color.a = 255;
    } else {
invalid_color:
        fprintf (stderr, "Missing or invalid field 'Game.color' in '%s'\n", filename);
    }

    toml_free (conf);
end:
    free (filename);
}
void
save_settings (void)
{
    char *filename = make_filename ();

    char *dup = strdup (filename);
    dirname (dup);
    mkdir_p (dup);
    free (dup);

    FILE *file = fopen (filename, "w");
    if (!file) {
        fprintf (stderr, "Failed to open '%s': %s\n", filename, strerror (errno));
        free (filename);
        return;
    }
    fputs ("[Game]\n", file);
    fprintf (file, "\tn_mines = %d\n", default_n_mines);
    fprintf (file, "\twidth = %d\n", default_width);
    fprintf (file, "\theight = %d\n", default_height);
    fprintf (file, "\tcolor = [ %d, %d, %d ]\n", default_color.r, default_color.g, default_color.b);
    fclose (file);
    free (filename);
}
