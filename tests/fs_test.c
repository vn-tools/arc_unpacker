#include <stdlib.h>
#include <string.h>
#include "assert_ex.h"
#include "fs.h"

void test_get_files()
{
    Array *result = get_files("tests/test_files/gfx");
    assert_equali(3, array_size(result));

    size_t correct = 0;
    size_t i;
    for (i = 0; i < 3; i ++)
    {
        char *actual_path = (char*)array_get(result, i);
        if (!strcmp("tests/test_files/gfx/reimu_transparent.png", actual_path)
            || !strcmp("tests/test_files/gfx/reimu_opaque.jpg", actual_path)
            || !strcmp("tests/test_files/gfx/usagi_opaque.png", actual_path))
        {
            ++ correct;
        }
        free(actual_path);
    }
    assert_equali(3, correct);
}

void test_get_files_recursive()
{
    Array *result = get_files_recursive("tests/test_files/gfx");
    assert_that(array_size(result) > 3);
}

void test_dirname(const char *expected, const char *input)
{
    char *dir = dirname(input);
    assert_equals(expected, dir);
    free(dir);
}

void test_is_dir(bool expected, const char *input)
{
    assert_equali(expected, is_dir(input));
}

int main(void)
{
    test_get_files();
    test_get_files_recursive();

    test_is_dir(false, "nonexisting");
    test_is_dir(true, "tests");
    test_is_dir(true, "tests/");
    test_is_dir(false, "tests/test_files/gfx/usagi_opaque.png");

    test_dirname("", "");
    test_dirname("/", "/");
    test_dirname("/", "/dir");
    test_dirname("/", "/dir/");
    test_dirname("/dir/", "/dir/sub");
    test_dirname("/dir/", "/dir/sub/");
    test_dirname("/dir/sub/", "/dir/sub/deep");
    test_dirname("/dir/sub/", "/dir/sub/deep/");
    test_dirname("dir/", "dir/sub");
    test_dirname("dir/", "dir/sub/");
    test_dirname("dir/sub/", "dir/sub/deep");
    test_dirname("dir/sub/", "dir/sub/deep/");
    test_dirname("\\", "\\");
    test_dirname("\\", "\\dir");
    test_dirname("\\", "\\dir\\");
    test_dirname("\\dir\\", "\\dir\\sub");
    test_dirname("\\dir\\", "\\dir\\sub\\");
    test_dirname("\\dir\\", "\\dir\\sub\\/");
    test_dirname("\\dir\\sub\\/", "\\dir\\sub\\/deep/");
    test_dirname("\\dir\\sub\\/deep\\", "\\dir\\sub\\/deep\\deepest");
    test_dirname("C:\\", "C:\\");
    test_dirname("C:\\", "C:\\dir");
    test_dirname("C:\\", "C:\\dir\\");
    test_dirname("C:\\dir\\", "C:\\dir\\sub");
    test_dirname("C:\\dir\\", "C:\\dir\\sub\\");
    test_dirname("C:\\dir\\sub\\", "C:\\dir\\sub\\deep");
    test_dirname("C:\\dir\\sub\\", "C:\\dir\\sub\\deep\\");
    return 0;
}
