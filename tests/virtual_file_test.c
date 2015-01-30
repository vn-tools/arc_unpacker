#include "assert_ex.h"
#include "virtual_file.h"

void test_empty_file()
{
    VirtualFile *file = vf_create();
    assert_null(vf_get_data(file));
    assert_null(vf_get_name(file));
    assert_equali(0, vf_get_size(file));
    vf_destroy(file);
}

void test_setting_name()
{
    VirtualFile *file = vf_create();
    assert_that(vf_set_name(file, "abc"));
    assert_equals("abc", vf_get_name(file));
    vf_destroy(file);
}

void test_setting_data()
{
    VirtualFile *file = vf_create();
    assert_that(vf_set_data(file, "abc", 3));
    assert_equals("abc", vf_get_data(file));
    assert_equali(3, vf_get_size(file));
    vf_destroy(file);
}

void test_changing_extension_without_extension()
{
    VirtualFile *file = vf_create();
    assert_that(vf_set_name(file, "abc"));
    vf_change_extension(file, "xyz");
    assert_equals("abc.xyz", vf_get_name(file));
}

void test_changing_extension_with_extension()
{
    VirtualFile *file = vf_create();
    assert_that(vf_set_name(file, "abc.de"));
    vf_change_extension(file, "xyz");
    assert_equals("abc.xyz", vf_get_name(file));
}

void test_changing_extension_with_extra_dots()
{
    VirtualFile *file = vf_create();
    assert_that(vf_set_name(file, "abc.de"));
    vf_change_extension(file, ".xyz");
    assert_equals("abc.xyz", vf_get_name(file));
}

int main(void)
{
    test_empty_file();
    test_setting_name();
    test_setting_data();
    test_changing_extension_without_extension();
    test_changing_extension_with_extension();
    test_changing_extension_with_extra_dots();
    return 0;
}
