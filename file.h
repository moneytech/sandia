#ifndef FILE_H
#define FILE_H

#include "basics.h"

char *slurp (const char *path);
void load_file (Atom env, const char *path);

#endif
