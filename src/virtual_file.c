#include <stdlib.h>
#include <string.h>
#include "assert_ex.h"
#include "virtual_file.h"

struct VirtualFile
{
    size_t size;
    char *data;
    char *name;
};

VirtualFile *vf_create()
{
    VirtualFile *file = (VirtualFile*)malloc(sizeof(VirtualFile));
    assert_not_null(file);
    file->size = 0;
    file->data = NULL;
    file->name = NULL;
    return file;
}

void vf_destroy(VirtualFile *file)
{
    assert_not_null(file);
    free(file->data);
    free(file->name);
    free(file);
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

void vf_change_extension(VirtualFile *file, const char *new_extension)
{
    assert_not_null(file);
    if (file->name == NULL)
        return;

    char *ptr = strrchr(file->name, '.');
    if (ptr != NULL)
        *ptr = '\0';

    while (new_extension[0] == '.')
        new_extension ++;

    size_t base_length = strlen(file->name);
    char *new_name = (char*)realloc(
        file->name,
        base_length + 1 + strlen(new_extension) + 1);

    assert_not_null(new_name);
    strcpy(new_name + base_length, ".");
    strcpy(new_name + base_length + 1, new_extension);
    file->name = new_name;
}

bool vf_set_name(VirtualFile *file, const char *new_name)
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
    file->data = (char*)malloc(length);
    if (file->data == NULL)
    {
        file->size = 0;
        return false;
    }
    else
    {
        memcpy(file->data, data, length);
        file->size = length;
        return true;
    }
}
