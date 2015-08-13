#include "arg_parser.h"
#include "test_support/catch.hh"

using namespace au;

TEST_CASE("Missing switches don't throw exceptions")
{
    ArgParser ap;
    REQUIRE(ap.get_switch("-s") == "");
    REQUIRE(ap.get_switch("--long") == "");
    REQUIRE(!ap.has_switch("-s"));
    REQUIRE(!ap.has_switch("--long"));
}

TEST_CASE("Switches are not confused with flags")
{
    ArgParser ap;
    ap.parse(std::vector<std::string>{"-f"});
    REQUIRE(ap.get_switch("-f") == "");
    REQUIRE(!ap.has_switch("-f"));
}

TEST_CASE("Short switches work")
{
    ArgParser ap;
    ap.parse(std::vector<std::string>{"-s=short"});
    REQUIRE(ap.get_switch("-s") == "short");
    REQUIRE(ap.get_switch("s") == "short");
    REQUIRE(ap.has_switch("-s"));
    REQUIRE(ap.has_switch("s"));
}

TEST_CASE("Long switches work")
{
    ArgParser ap;
    ap.parse(std::vector<std::string>{"--long=long"});
    REQUIRE(ap.get_switch("--long") == "long");
    REQUIRE(ap.get_switch("-long") == "long");
    REQUIRE(ap.get_switch("long") == "long");
    REQUIRE(ap.has_switch("--long"));
    REQUIRE(ap.has_switch("-long"));
    REQUIRE(ap.has_switch("long"));
}

#if 0
TEST_CASE("Short switches are overriden with later values")
{
    ArgParser ap;
    ap.parse(std::vector<std::string>{"-s=short1", "-s=short2"});
    REQUIRE(ap.get_switch("-s") == "short2");
}

TEST_CASE("Long switches are overriden with later values")
{
    ArgParser ap;
    ap.parse(std::vector<std::string>{"--long=long1", "--long=long2"});
    REQUIRE(ap.get_switch("--long") == "long2");
}
#endif

TEST_CASE("Switches with values containing spaces work")
{
    ArgParser ap;
    ap.parse(std::vector<std::string>{"--switch=long switch"});
    REQUIRE(ap.get_switch("--switch") == "long switch");
}

TEST_CASE("Switches with empty values work")
{
    ArgParser ap;
    ap.parse(std::vector<std::string>{"--switch="});
    REQUIRE(ap.get_switch("--switch") == "");
}

TEST_CASE("Missing flags don't throw exceptions")
{
    ArgParser ap;
    REQUIRE(!ap.has_flag("nope"));
}

TEST_CASE("Basic flags work")
{
    ArgParser ap;
    ap.parse(std::vector<std::string>{"--flag"});
    REQUIRE(ap.has_flag("flag"));
}

TEST_CASE("Flags mixed with stray arguments are not confused")
{
    ArgParser ap;
    ap.parse(std::vector<std::string>{"--flag", "stray"});
    REQUIRE(ap.has_flag("flag"));
    auto stray = ap.get_stray();
    REQUIRE(stray.size() == 1);
    REQUIRE(stray[0] == "stray");
}

TEST_CASE("Missing stray arguments return empty list")
{
    ArgParser ap;
    auto stray = ap.get_stray();
    REQUIRE(stray.size() == 0);
}

TEST_CASE("Basic stray arguments work")
{
    ArgParser ap;
    ap.parse(std::vector<std::string>{"stray1", "stray2"});
    auto stray = ap.get_stray();
    REQUIRE(stray.size() == 2);
    REQUIRE(stray[0] == "stray1");
    REQUIRE(stray[1] == "stray2");
}

TEST_CASE("Stray arguments with spaces work")
{
    ArgParser ap;
    ap.parse(std::vector<std::string>{"long stray"});
    auto stray = ap.get_stray();
    REQUIRE(stray.size() == 1);
    REQUIRE(stray[0] == "long stray");
}

TEST_CASE("Mixed types of arguments work")
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

    REQUIRE(ap.get_switch("--switch") == "s");
    REQUIRE(ap.has_flag("flag1"));
    REQUIRE(ap.has_flag("flag2"));

    auto stray = ap.get_stray();
    REQUIRE(stray.size() == 2);
    REQUIRE(stray[0] == "stray1");
    REQUIRE(stray[1] == "stray2");
}
