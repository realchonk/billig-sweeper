#ifndef FILE_BSW_UTIL_H
#define FILE_BSW_UTIL_H

// Generate a random number between `min_val` and `max_val`.
int rrand (int min_val, int max_val);

// Create a path relative to the executable.
char *relative_path (const char *path);

// pow() for unsigned integers
unsigned powui (unsigned base, unsigned exponent);

#endif // FILE_BSW_UTIL_H
