#include "arg_parser.h"
#include "test_support/catch.h"

using namespace au;

TEST_CASE("ArgParser", "[core]")
{
    SECTION("Set short switches retrieval and querying")
    {
        ArgParser ap;
        ap.register_switch({"-s"});
        ap.parse(std::vector<std::string>{"-s=short"});
        REQUIRE(ap.get_switch("s") == "short");
        REQUIRE(ap.has_switch("s"));
    }

    SECTION("Set long switches retrival and querying")
    {
        ArgParser ap;
        ap.register_switch({"--long"});
        ap.parse(std::vector<std::string>{"--long=long"});
        REQUIRE(ap.get_switch("long") == "long");
        REQUIRE(ap.has_switch("long"));
    }

    SECTION("Unset short switches retrival and querying")
    {
        ArgParser ap;
        ap.register_switch({"-s"});
        ap.parse(std::vector<std::string>{});
        REQUIRE(ap.get_switch("s") == "");
        REQUIRE(!ap.has_switch("s"));
    }

    SECTION("Unset long switches retrival and querying")
    {
        ArgParser ap;
        ap.register_switch({"--long"});
        ap.parse(std::vector<std::string>{});
        REQUIRE(ap.get_switch("long") == "");
        REQUIRE(!ap.has_switch("long"));
    }

    SECTION("Querying undefined switches throws exceptions")
    {
        ArgParser ap;
        ap.parse(std::vector<std::string>{});
        REQUIRE_THROWS(ap.get_switch("long"));
        REQUIRE_THROWS(ap.has_switch("long"));
    }

    SECTION("Switch retrieval using arbitrary number of hyphens")
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

    SECTION("Switches are not confused with flags")
    {
        ArgParser ap;
        ap.register_flag({"-f"});
        ap.parse(std::vector<std::string>{"-f"});
        REQUIRE_THROWS(ap.get_switch("-f"));
        REQUIRE_THROWS(ap.has_switch("-f"));
    }

    SECTION("Short switches are overriden with later values")
    {
        ArgParser ap;
        ap.register_switch({"-s"});
        ap.parse(std::vector<std::string>{"-s=short1", "-s=short2"});
        REQUIRE(ap.get_switch("-s") == "short2");
    }

    SECTION("Long switches are overriden with later values")
    {
        ArgParser ap;
        ap.register_switch({"--long"});
        ap.parse(std::vector<std::string>{"--long=long1", "--long=long2"});
        REQUIRE(ap.get_switch("--long") == "long2");
    }

    SECTION("Switches with values containing spaces")
    {
        ArgParser ap;
        ap.register_switch({"--switch"});
        ap.parse(std::vector<std::string>{"--switch=long switch"});
        REQUIRE(ap.get_switch("--switch") == "long switch");
    }

    SECTION("Switches with empty values")
    {
        ArgParser ap;
        ap.register_switch({"--switch"});
        ap.parse(std::vector<std::string>{"--switch="});
        REQUIRE(ap.get_switch("--switch") == "");
    }

    SECTION("Set flags")
    {
        ArgParser ap;
        ap.register_flag({"--flag"});
        ap.parse(std::vector<std::string>{"--flag"});
        REQUIRE(ap.has_flag("flag"));
    }

    SECTION("Unset flags")
    {
        ArgParser ap;
        ap.register_flag({"--flag"});
        ap.parse(std::vector<std::string>{});
        REQUIRE(!ap.has_flag("flag"));
    }

    SECTION("Querying undefined flags throws exceptions")
    {
        ArgParser ap;
        REQUIRE_THROWS(!ap.has_flag("nope"));
    }

    SECTION("Flag retrieval using arbitrary number of hyphens")
    {
        ArgParser ap;
        ap.register_flag({"--flag"});
        ap.parse(std::vector<std::string>{"--flag"});
        REQUIRE(ap.has_flag("--flag"));
        REQUIRE(ap.has_flag("-flag"));
        REQUIRE(ap.has_flag("flag"));
    }

    SECTION("Flags mixed with stray arguments are not confused")
    {
        ArgParser ap;
        ap.register_flag({"--flag"});
        ap.parse(std::vector<std::string>{"--flag", "stray"});
        REQUIRE(ap.has_flag("flag"));
        REQUIRE_THROWS(ap.has_switch("flag"));
        REQUIRE_THROWS(ap.get_switch("flag"));
        const auto stray = ap.get_stray();
        REQUIRE(stray.size() == 1);
        REQUIRE(stray[0] == "stray");
    }

    SECTION("Defining the same flag or switch twice throws exception")
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

    SECTION("Basic stray arguments")
    {
        ArgParser ap;
        ap.parse(std::vector<std::string>{"stray1", "stray2"});
        const auto stray = ap.get_stray();
        REQUIRE(stray.size() == 2);
        REQUIRE(stray[0] == "stray1");
        REQUIRE(stray[1] == "stray2");
    }

    SECTION("Stray arguments with spaces")
    {
        ArgParser ap;
        ap.parse(std::vector<std::string>{"long stray"});
        const auto stray = ap.get_stray();
        REQUIRE(stray.size() == 1);
        REQUIRE(stray[0] == "long stray");
    }

    SECTION("Empty stray arguments")
    {
        ArgParser ap;
        const auto stray = ap.get_stray();
        REQUIRE(stray.size() == 0);
    }

    SECTION("Mixed types of arguments")
    {
        ArgParser ap;
        ap.register_switch({"--switch"});
        ap.register_flag({"--flag1"});
        ap.register_flag({"--flag2"});
        const std::vector<std::string> args
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

        const auto stray = ap.get_stray();
        REQUIRE(stray.size() == 2);
        REQUIRE(stray[0] == "stray1");
        REQUIRE(stray[1] == "stray2");
    }
}
