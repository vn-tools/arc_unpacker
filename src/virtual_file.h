#ifndef VIRTUAL_FILE_H
#define VIRTUAL_FILE_H
#include <stddef.h>
#include <stdbool.h>

typedef struct VirtualFile VirtualFile;

VirtualFile *vf_create();
VirtualFile *vf_create_from_hdd(const char *path);
void vf_destroy(VirtualFile *file);

char *vf_get_data(VirtualFile *file);
size_t vf_get_size(VirtualFile *file);
char *vf_get_name(VirtualFile *file);
void vf_change_extension(VirtualFile *file, const char *new_extension);

bool vf_set_name(VirtualFile *file, const char *new_name);
bool vf_set_data(VirtualFile *file, const char *new_data, size_t length);

#endif
