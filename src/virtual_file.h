#ifndef VIRTUAL_FILE_H
#define VIRTUAL_FILE_H
#include <stddef.h>
#include <stdbool.h>
#include "io.h"

typedef struct
{
    IO *io;

    void *internals;
} VirtualFile;

VirtualFile *virtual_file_create();
void virtual_file_destroy(VirtualFile *file);

const char *virtual_file_get_name(VirtualFile *file);
bool virtual_file_set_name(VirtualFile *file, const char *new_name);
void virtual_file_change_extension(VirtualFile *file, const char *new_ext);

#endif
