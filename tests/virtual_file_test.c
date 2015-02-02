#include "assert_ex.h"
#include "virtual_file.h"

void test_empty_file()
{
    VirtualFile *file = virtual_file_create();
    assert_not_null(file->io);
    assert_null(virtual_file_get_name(file));
    virtual_file_destroy(file);
}

void test_setting_name()
{
    VirtualFile *file = virtual_file_create();
    assert_that(virtual_file_set_name(file, "abc"));
    assert_equals("abc", virtual_file_get_name(file));
    virtual_file_destroy(file);
}

void test_changing_extension_null()
{
    VirtualFile *file = virtual_file_create();
    virtual_file_change_extension(file, "xyz");
    assert_null(virtual_file_get_name(file));
}

void test_changing_extension_without_extension()
{
    VirtualFile *file = virtual_file_create();
    assert_that(virtual_file_set_name(file, "abc"));
    virtual_file_change_extension(file, "xyz");
    assert_equals("abc.xyz", virtual_file_get_name(file));
}

void test_changing_extension_with_extension()
{
    VirtualFile *file = virtual_file_create();
    assert_that(virtual_file_set_name(file, "abc.de"));
    virtual_file_change_extension(file, "xyz");
    assert_equals("abc.xyz", virtual_file_get_name(file));
}

void test_changing_extension_with_extra_dots()
{
    VirtualFile *file = virtual_file_create();
    assert_that(virtual_file_set_name(file, "abc.de"));
    virtual_file_change_extension(file, ".xyz");
    assert_equals("abc.xyz", virtual_file_get_name(file));
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
