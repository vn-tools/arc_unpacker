#include "test_support/eassert.h"
#include "file.h"

void test_empty_file()
{
    File file;
    eassert(file.name == "");
}

void test_setting_name()
{
    File file;
    file.name = "abc";
    eassert(file.name == "abc");
}

void test_changing_extension(
    const std::string name,
    const std::string new_extension,
    const std::string expected_name)
{
    File file;
    file.name = name;
    file.change_extension(new_extension);
    eassert(file.name == expected_name);
}

int main(void)
{
    test_empty_file();
    test_setting_name();
    test_changing_extension("",            "xyz",  "");
    test_changing_extension(".",           "xyz",  ".");
    test_changing_extension("..",          "xyz",  "..");
    test_changing_extension("abc",         "xyz",  "abc.xyz");
    test_changing_extension("abc.de",      "xyz",  "abc.xyz");
    test_changing_extension("abc.de",      ".xyz", "abc.xyz");
    test_changing_extension("abc.",        "xyz",  "abc.xyz");
    test_changing_extension(".abc.",       "xyz",  ".abc.xyz");
    test_changing_extension("./abc/",      "xyz",  "./abc/");
    test_changing_extension("./abc/.",     "xyz",  "./abc/.");
    test_changing_extension("./abc/..",    "xyz",  "./abc/..");
    test_changing_extension("./abc/def",   "xyz",  "./abc/def.xyz");
    test_changing_extension("./abc/def.",  "xyz",  "./abc/def.xyz");
    test_changing_extension("./abc/.def.", "xyz",  "./abc/.def.xyz");
    return 0;
}
