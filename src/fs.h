#ifndef MKPATH_H
#define MKPATH_H
#include <stdbool.h>
#include "collections/array.h"

bool is_dir(const char *path);
Array *get_files_recursive(const char *path);
Array *get_files(const char *path);
char *basename(const char *path);
char *dirname(const char *path);
bool mkpath(const char *path);

#endif
