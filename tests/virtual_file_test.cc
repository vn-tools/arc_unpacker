#include <assert.h>
#include <string.h>
#include "virtual_file.h"

void test_empty_file()
{
    VirtualFile *file = virtual_file_create();
    assert(file->io != NULL);
    assert(virtual_file_get_name(file) == NULL);
    virtual_file_destroy(file);
}

void test_setting_name()
{
    VirtualFile *file = virtual_file_create();
    assert(virtual_file_set_name(file, "abc"));
    assert(strcmp("abc", virtual_file_get_name(file)) == 0);
    virtual_file_destroy(file);
}

void test_changing_extension_null()
{
    VirtualFile *file = virtual_file_create();
    virtual_file_change_extension(file, "xyz");
    assert(virtual_file_get_name(file) == NULL);
}

void test_changing_extension_without_extension()
{
    VirtualFile *file = virtual_file_create();
    assert(virtual_file_set_name(file, "abc"));
    virtual_file_change_extension(file, "xyz");
    assert(strcmp("abc.xyz", virtual_file_get_name(file)) == 0);
}

void test_changing_extension_with_extension()
{
    VirtualFile *file = virtual_file_create();
    assert(virtual_file_set_name(file, "abc.de"));
    virtual_file_change_extension(file, "xyz");
    assert(strcmp("abc.xyz", virtual_file_get_name(file)) == 0);
}

void test_changing_extension_with_extra_dots()
{
    VirtualFile *file = virtual_file_create();
    assert(virtual_file_set_name(file, "abc.de"));
    virtual_file_change_extension(file, ".xyz");
    assert(strcmp("abc.xyz", virtual_file_get_name(file)) == 0);
}

int main(void)
{
    test_empty_file();
    test_setting_name();
    test_changing_extension_null();
    test_changing_extension_without_extension();
    test_changing_extension_with_extension();
    test_changing_extension_with_extra_dots();
    return 0;
}
