#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "assert.h"
#include "virtual_file.h"

struct VirtualFile
{
    char *data;
    size_t size;
    char *name;
};

VirtualFile *vf_create()
{
    VirtualFile *file = malloc(sizeof(VirtualFile));
    file->data = NULL;
    file->size = 0;
    file->name = NULL;
    return file;
}

void vf_destroy(VirtualFile *file)
{
    assert_not_null(file);
    free(file->data);
    free(file->name);
}

char *vf_get_data(VirtualFile *file)
{
    return file->data;
}

size_t vf_get_size(VirtualFile *file)
{
    return file->size;
}

char *vf_get_name(VirtualFile *file)
{
    return file->name;
}

bool vf_change_extension(VirtualFile *file, const char *new_extension)
{
    char *new_name;
    size_t base_length;
    char *ptr = strrchr(file->name, '.');
    assert_not_null(file);
    assert_not_null(file->name);
    if (ptr != NULL)
        *ptr = '\0';

    while (new_extension[0] == '.')
        new_extension ++;

    base_length = strlen(file->name);
    new_name = realloc(
        file->name,
        base_length + 1 + strlen(new_extension) + 1);

    if (!new_name)
    {
        errno = ENOMEM;
        return false;
    }

    strcpy(new_name + base_length, ".");
    strcpy(new_name + base_length + 1, new_extension);
    file->name = new_name;
    return true;
}

bool vf_set_name(VirtualFile *file, const char *const new_name)
{
    assert_not_null(file);
    if (file->name != NULL)
        free(file->name);
    file->name = strdup(new_name);
    return file->name != NULL;
}

bool vf_set_data(VirtualFile *file, const char *data, size_t length)
{
    assert_not_null(file);
    if (file->data != NULL)
        free(file->data);
    file->data = strndup(data, length);
    if (file->data == NULL)
    {
        file->size = 0;
        return false;
    }
    else
    {
        file->size = length;
        return true;
    }
}
