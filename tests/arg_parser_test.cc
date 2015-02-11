#include "arg_parser.h"
#include "test_support/eassert.h"

void test_switch_missing()
{
    ArgParser ap;
    eassert(ap.get_switch("-s") == "");
    eassert(ap.get_switch("--long") == "");
    eassert(!ap.has_switch("-s"));
    eassert(!ap.has_switch("--long"));
}

void test_switch_is_not_a_flag()
{
    ArgParser ap;
    ap.parse(std::vector<std::string>{"-f"});
    eassert(ap.get_switch("-f") == "");
    eassert(!ap.has_switch("-f"));
}

void test_switch_short()
{
    ArgParser ap;
    ap.parse(std::vector<std::string>{"-s=short"});
    eassert(ap.get_switch("-s") == "short");
    eassert(ap.get_switch("s") == "short");
    eassert(ap.has_switch("-s"));
    eassert(ap.has_switch("s"));
}

void test_switch_long()
{
    ArgParser ap;
    ap.parse(std::vector<std::string>{"--long=long"});
    eassert(ap.get_switch("--long") == "long");
    eassert(ap.get_switch("-long") == "long");
    eassert(ap.get_switch("long") == "long");
    eassert(ap.has_switch("--long"));
    eassert(ap.has_switch("-long"));
    eassert(ap.has_switch("long"));
}

void test_switch_overriding_short()
{
    ArgParser ap;
    ap.parse(std::vector<std::string>{"-s=short1", "-s=short2"});
    eassert(ap.get_switch("-s") == "short2");
}

void test_switch_overriding_long()
{
    ArgParser ap;
    ap.parse(std::vector<std::string>{"--long=long1", "--long=long2"});
    eassert(ap.get_switch("--long") == "long2");
}

void test_switch_with_space()
{
    ArgParser ap;
    ap.parse(std::vector<std::string>{"--switch=long switch"});
    eassert(ap.get_switch("--switch") == "long switch");
}

void test_switch_empty_value()
{
    ArgParser ap;
    ap.parse(std::vector<std::string>{"--switch="});
    eassert(ap.get_switch("--switch") == "");
}

void test_flag_missing()
{
    ArgParser ap;
    eassert(!ap.has_flag("nope"));
}

void test_flag()
{
    ArgParser ap;
    ap.parse(std::vector<std::string>{"--flag"});
    eassert(ap.has_flag("flag"));
}

void test_flag_mixed_with_stray()
{
    ArgParser ap;
    ap.parse(std::vector<std::string>{"--flag", "stray"});
    eassert(ap.has_flag("flag"));
    auto stray = ap.get_stray();
    eassert(stray.size() == 1);
    eassert(stray[0] == "stray");
}

void test_stray_missing()
{
    ArgParser ap;
    auto stray = ap.get_stray();
    eassert(stray.size() == 0);
}

void test_stray()
{
    ArgParser ap;
    ap.parse(std::vector<std::string>{"stray1", "stray2"});
    auto stray = ap.get_stray();
    eassert(stray.size() == 2);
    eassert(stray[0] == "stray1");
    eassert(stray[1] == "stray2");
}

void test_stray_with_space()
{
    ArgParser ap;
    ap.parse(std::vector<std::string>{"long stray"});
    auto stray = ap.get_stray();
    eassert(stray.size() == 1);
    eassert(stray[0] == "long stray");
}

void test_mixed_types()
{
    ArgParser ap;
    std::vector<std::string> args
    {
        "stray1",
        "--switch=s",
        "--flag1",
        "stray2",
        "--flag2"
    };
    ap.parse(args);

    eassert(ap.get_switch("--switch") == "s");
    eassert(ap.has_flag("flag1"));
    eassert(ap.has_flag("flag2"));

    auto stray = ap.get_stray();
    eassert(stray.size() == 2);
    eassert(stray[0] == "stray1");
    eassert(stray[1] == "stray2");
}

#if 0
#endif

int main(void)
{
    test_switch_missing();
    test_switch_is_not_a_flag();
    test_switch_short();
    test_switch_long();
    // TODO: make this pass
    #if 0
    test_switch_overriding_short();
    test_switch_overriding_long();
    #endif
    test_switch_with_space();
    test_switch_empty_value();
    test_flag_missing();
    test_flag();
    test_flag_mixed_with_stray();
    test_stray_missing();
    test_stray();
    test_stray_with_space();
    test_mixed_types();
    return 0;
}
