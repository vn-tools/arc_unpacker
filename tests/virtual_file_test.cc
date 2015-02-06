#include <assert.h>
#include <string.h>
#include "virtual_file.h"

void test_empty_file()
{
    VirtualFile file;
    assert(file.name == "");
}

void test_setting_name()
{
    VirtualFile file;
    file.name = "abc";
    assert(file.name == "abc");
}

void test_changing_extension_null()
{
    VirtualFile file;
    file.change_extension("xyz");
    assert(file.name == "");
}

void test_changing_extension_without_extension()
{
    VirtualFile file;
    file.name = "abc";
    file.change_extension("xyz");
    assert(file.name == "abc.xyz");
}

void test_changing_extension_with_extension()
{
    VirtualFile file;
    file.name = "abc.de";
    file.change_extension("xyz");
    assert(file.name == "abc.xyz");
}

void test_changing_extension_with_extra_dots()
{
    VirtualFile file;
    file.name = "abc.de";
    file.change_extension(".xyz");
    assert(file.name == "abc.xyz");
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
