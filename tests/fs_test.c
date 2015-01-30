#include <stdlib.h>
#include "assert_ex.h"
#include "fs.h"

void test_dirname(const char *expected, const char *input)
{
    char *dir = dirname(input);
    assert_equals(expected, dir);
    free(dir);
}

int main(void)
{
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
