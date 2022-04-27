#ifndef FILE_BSW_UTIL_H
#define FILE_BSW_UTIL_H

// Generate a random number between `min_val` and `max_val`.
int rrand (int min_val, int max_val);

// Create a path relative to the executable.
char *relative_path (const char *path);

// pow() for unsigned integers
unsigned powui (unsigned base, unsigned exponent);

// Opens a URL in a browser.
void open_url (const char *url);

#endif // FILE_BSW_UTIL_H
