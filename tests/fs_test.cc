#include <algorithm>
#include "fs.h"
#include "test_support/eassert.h"

void test_get_files()
{
    auto result = get_files("tests/test_files/gfx");
    eassert(result.size() == 3);

    eassert(std::find(
        result.begin(),
        result.end(),
        "tests/test_files/gfx/reimu_transparent.png") != result.end());

    eassert(std::find(
        result.begin(),
        result.end(),
        "tests/test_files/gfx/reimu_opaque.jpg") != result.end());

    eassert(std::find(
        result.begin(),
        result.end(),
        "tests/test_files/gfx/usagi_opaque.png") != result.end());
}

void test_get_files_recursive()
{
    auto result = get_files_recursive("tests/test_files/gfx");
    eassert(result.size() > 3);
}

void test_basename(const std::string expected, const std::string input)
{
    std::string dir = basename(input);
    eassert(dir == expected);
}

void test_dirname(const std::string expected, const std::string input)
{
    std::string dir = dirname(input);
    eassert(dir == expected);
}

void test_is_dir(bool expected, const std::string input)
{
    eassert(is_dir(input) == expected);
}

int main(void)
{
    test_get_files();
    test_get_files_recursive();

    test_is_dir(false, "nonexisting");
    test_is_dir(true, "tests");
    test_is_dir(true, "tests/");
    test_is_dir(false, "tests/test_files/gfx/usagi_opaque.png");

    test_basename("",         "");
    test_basename("",         "/");
    test_basename("",         "Z:/");
    test_basename("test.txt", "test.txt");
    test_basename("test.txt", "/test.txt");
    test_basename("test.txt", "Z:/test.txt");
    test_basename("test.txt", "dir/test.txt");
    test_basename("test.txt", "/dir/test.txt");
    test_basename("test.txt", "Z:/dir/test.txt");

    test_dirname("",                    "");
    test_dirname("",                    "file");
    test_dirname("/",                   "/");
    test_dirname("/",                   "/dir");
    test_dirname("/",                   "/dir/");
    test_dirname("/dir/",               "/dir/sub");
    test_dirname("/dir/",               "/dir/sub/");
    test_dirname("/dir/sub/",           "/dir/sub/deep");
    test_dirname("/dir/sub/",           "/dir/sub/deep/");
    test_dirname("dir/",                "dir/sub");
    test_dirname("dir/",                "dir/sub/");
    test_dirname("dir/sub/",            "dir/sub/deep");
    test_dirname("dir/sub/",            "dir/sub/deep/");
    test_dirname("\\",                  "\\");
    test_dirname("\\",                  "\\dir");
    test_dirname("\\",                  "\\dir\\");
    test_dirname("\\dir\\",             "\\dir\\sub");
    test_dirname("\\dir\\",             "\\dir\\sub\\");
    test_dirname("\\dir\\",             "\\dir\\sub\\/");
    test_dirname("\\dir\\sub\\/",       "\\dir\\sub\\/deep/");
    test_dirname("\\dir\\sub\\/deep\\", "\\dir\\sub\\/deep\\deepest");
    test_dirname("C:\\",                "C:\\");
    test_dirname("C:\\",                "C:\\dir");
    test_dirname("C:\\",                "C:\\dir\\");
    test_dirname("C:\\dir\\",           "C:\\dir\\sub");
    test_dirname("C:\\dir\\",           "C:\\dir\\sub\\");
    test_dirname("C:\\dir\\sub\\",      "C:\\dir\\sub\\deep");
    test_dirname("C:\\dir\\sub\\",      "C:\\dir\\sub\\deep\\");
    return 0;
}
