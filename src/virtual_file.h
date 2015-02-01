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

VirtualFile *vf_create();
void vf_destroy(VirtualFile *file);

const char *vf_get_name(VirtualFile *file);
bool vf_set_name(VirtualFile *file, const char *new_name);
void vf_change_extension(VirtualFile *file, const char *new_extension);

#endif
