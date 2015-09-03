#include "arg_parser.h"
#include "test_support/catch.hh"

using namespace au;

TEST_CASE("Set short switches retrieval and querying", "[arg_parser]")
{
    ArgParser ap;
    ap.register_switch({"-s"});
    ap.parse(std::vector<std::string>{"-s=short"});
    REQUIRE(ap.get_switch("s") == "short");
    REQUIRE(ap.has_switch("s"));
}

TEST_CASE("Set long switches retrival and querying", "[arg_parser]")
{
    ArgParser ap;
    ap.register_switch({"--long"});
    ap.parse(std::vector<std::string>{"--long=long"});
    REQUIRE(ap.get_switch("long") == "long");
    REQUIRE(ap.has_switch("long"));
}

TEST_CASE("Unset short switches retrival and querying", "[arg_parser]")
{
    ArgParser ap;
    ap.register_switch({"-s"});
    ap.parse(std::vector<std::string>{});
    REQUIRE(ap.get_switch("s") == "");
    REQUIRE(!ap.has_switch("s"));
}

TEST_CASE("Unset long switches retrival and querying", "[arg_parser]")
{
    ArgParser ap;
    ap.register_switch({"--long"});
    ap.parse(std::vector<std::string>{});
    REQUIRE(ap.get_switch("long") == "");
    REQUIRE(!ap.has_switch("long"));
}

TEST_CASE("Querying undefined switches throws exceptions", "[arg_parser]")
{
    ArgParser ap;
    ap.parse(std::vector<std::string>{});
    REQUIRE_THROWS(ap.get_switch("long"));
    REQUIRE_THROWS(ap.has_switch("long"));
}

TEST_CASE("Switch retrieval using arbitrary number of hyphens", "[arg_parser]")
{
    ArgParser ap;
    ap.register_switch({"--switch"});
    ap.parse(std::vector<std::string>{"--switch=long switch"});
    REQUIRE(ap.get_switch("--switch") == "long switch");
    REQUIRE(ap.get_switch("-switch") == "long switch");
    REQUIRE(ap.get_switch("switch") == "long switch");
    REQUIRE(ap.has_switch("--switch"));
    REQUIRE(ap.has_switch("-switch"));
    REQUIRE(ap.has_switch("switch"));
}

TEST_CASE("Switches are not confused with flags", "[arg_parser]")
{
    ArgParser ap;
    ap.register_flag({"-f"});
    ap.parse(std::vector<std::string>{"-f"});
    REQUIRE_THROWS(ap.get_switch("-f"));
    REQUIRE_THROWS(ap.has_switch("-f"));
}

TEST_CASE("Short switches are overriden with later values", "[arg_parser]")
{
    ArgParser ap;
    ap.register_switch({"-s"});
    ap.parse(std::vector<std::string>{"-s=short1", "-s=short2"});
    REQUIRE(ap.get_switch("-s") == "short2");
}

TEST_CASE("Long switches are overriden with later values", "[arg_parser]")
{
    ArgParser ap;
    ap.register_switch({"--long"});
    ap.parse(std::vector<std::string>{"--long=long1", "--long=long2"});
    REQUIRE(ap.get_switch("--long") == "long2");
}

TEST_CASE("Switches with values containing spaces", "[arg_parser]")
{
    ArgParser ap;
    ap.register_switch({"--switch"});
    ap.parse(std::vector<std::string>{"--switch=long switch"});
    REQUIRE(ap.get_switch("--switch") == "long switch");
}

TEST_CASE("Switches with empty values", "[arg_parser]")
{
    ArgParser ap;
    ap.register_switch({"--switch"});
    ap.parse(std::vector<std::string>{"--switch="});
    REQUIRE(ap.get_switch("--switch") == "");
}

TEST_CASE("Set flags", "[arg_parser]")
{
    ArgParser ap;
    ap.register_flag({"--flag"});
    ap.parse(std::vector<std::string>{"--flag"});
    REQUIRE(ap.has_flag("flag"));
}

TEST_CASE("Unset flags", "[arg_parser]")
{
    ArgParser ap;
    ap.register_flag({"--flag"});
    ap.parse(std::vector<std::string>{});
    REQUIRE(!ap.has_flag("flag"));
}

TEST_CASE("Querying undefined flags throws exceptions", "[arg_parser]")
{
    ArgParser ap;
    REQUIRE_THROWS(!ap.has_flag("nope"));
}

TEST_CASE("Flag retrieval using arbitrary number of hyphens", "[arg_parser]")
{
    ArgParser ap;
    ap.register_flag({"--flag"});
    ap.parse(std::vector<std::string>{"--flag"});
    REQUIRE(ap.has_flag("--flag"));
    REQUIRE(ap.has_flag("-flag"));
    REQUIRE(ap.has_flag("flag"));
}

TEST_CASE("Flags mixed with stray arguments are not confused", "[arg_parser]")
{
    ArgParser ap;
    ap.register_flag({"--flag"});
    ap.parse(std::vector<std::string>{"--flag", "stray"});
    REQUIRE(ap.has_flag("flag"));
    REQUIRE_THROWS(ap.has_switch("flag"));
    REQUIRE_THROWS(ap.get_switch("flag"));
    auto stray = ap.get_stray();
    REQUIRE(stray.size() == 1);
    REQUIRE(stray[0] == "stray");
}

TEST_CASE(
    "Defining the same flag or switch twice throws exception", "[arg_parser]")
{
    ArgParser ap;
    ap.register_flag({"-s1", "--long1"});
    ap.register_switch({"-s2", "--long2"});
    REQUIRE_THROWS(ap.register_flag({"-s1"}));
    REQUIRE_THROWS(ap.register_flag({"--long1"}));
    REQUIRE_THROWS(ap.register_flag({"-s2"}));
    REQUIRE_THROWS(ap.register_flag({"--long2"}));
    REQUIRE_THROWS(ap.register_switch({"-s1"}));
    REQUIRE_THROWS(ap.register_switch({"--long1"}));
    REQUIRE_THROWS(ap.register_switch({"-s2"}));
    REQUIRE_THROWS(ap.register_switch({"--long2"}));
}

TEST_CASE("Basic stray arguments", "[arg_parser]")
{
    ArgParser ap;
    ap.parse(std::vector<std::string>{"stray1", "stray2"});
    auto stray = ap.get_stray();
    REQUIRE(stray.size() == 2);
    REQUIRE(stray[0] == "stray1");
    REQUIRE(stray[1] == "stray2");
}

TEST_CASE("Stray arguments with spaces", "[arg_parser]")
{
    ArgParser ap;
    ap.parse(std::vector<std::string>{"long stray"});
    auto stray = ap.get_stray();
    REQUIRE(stray.size() == 1);
    REQUIRE(stray[0] == "long stray");
}

TEST_CASE("Empty stray arguments", "[arg_parser]")
{
    ArgParser ap;
    auto stray = ap.get_stray();
    REQUIRE(stray.size() == 0);
}

TEST_CASE("Mixed types of arguments", "[arg_parser]")
{
    ArgParser ap;
    ap.register_switch({"--switch"});
    ap.register_flag({"--flag1"});
    ap.register_flag({"--flag2"});
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
