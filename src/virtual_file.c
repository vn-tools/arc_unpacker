#include <stdlib.h>
#include <string.h>
#include "assert_ex.h"
#include "logger.h"
#include "io.h"
#include "virtual_file.h"

typedef struct
{
    char *name;
} Internals;

VirtualFile *vf_create()
{
    VirtualFile *file = (VirtualFile*)malloc(sizeof(VirtualFile));
    assert_not_null(file);

    file->io = io_create_empty();
    assert_not_null(file->io);

    Internals *internals = (Internals*)malloc(sizeof(Internals));;
    assert_not_null(internals);
    internals->name = NULL;
    file->internals = internals;

    return file;
}

void vf_destroy(VirtualFile *file)
{
    assert_not_null(file);
    io_destroy(file->io);
    free(((Internals*)file->internals)->name);
    free(file);
}

void vf_change_extension(VirtualFile *file, const char *new_extension)
{
    char *name = ((Internals*)file->internals)->name;
    assert_not_null(file);
    if (name == NULL)
        return;

    char *ptr = strrchr(name, '.');
    if (ptr != NULL)
        *ptr = '\0';

    while (new_extension[0] == '.')
        new_extension ++;

    size_t base_length = strlen(name);
    char *new_name = (char*)realloc(
        name,
        base_length + 1 + strlen(new_extension) + 1);

    assert_not_null(new_name);
    strcpy(new_name + base_length, ".");
    strcpy(new_name + base_length + 1, new_extension);
    ((Internals*)file->internals)->name = new_name;
}

const char *vf_get_name(VirtualFile *file)
{
    return ((Internals*)file->internals)->name;
}

bool vf_set_name(VirtualFile *file, const char *new_name)
{
    char *name = ((Internals*)file->internals)->name;
    assert_not_null(file);
    if (name != NULL)
        free(name);
    name = strdup(new_name);
    ((Internals*)file->internals)->name = name;
    return name != NULL;
}
