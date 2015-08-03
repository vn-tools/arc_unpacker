#include <boost/filesystem.hpp>
#include "arc_unpacker.h"
#include "test_support/catch.hpp"
#include "test_support/suppress_output.h"

using namespace au;

TEST_CASE("Converting single files with CLI facade works")
{
    suppress_output([&]()
    {
        ArgParser arg_parser;
        arg_parser.parse({
            "path-to-self",
            "./tests/fmt/key/files/AYU_03.g00",
            "--fmt=key/g00" });
        ArcUnpacker arc_unpacker(arg_parser, "0.0");
        arc_unpacker.run();

        REQUIRE(boost::filesystem::is_regular_file("./AYU_03.png"));
        boost::filesystem::remove("./AYU_03.png");
    });
}

TEST_CASE("Unpacking archives with CLI facade works")
{
    suppress_output([&]()
    {
        ArgParser arg_parser;
        arg_parser.parse({
            "path-to-self",
            "./tests/fmt/kirikiri/files/xp3-v2.xp3",
            "--fmt=krkr/xp3",
            "--plugin=noop" });
        ArcUnpacker arc_unpacker(arg_parser, "0.0");
        arc_unpacker.run();

        REQUIRE(boost::filesystem::is_directory("./xp3-v2~.xp3"));
        REQUIRE(boost::filesystem::is_regular_file("./xp3-v2~.xp3/abc.txt"));
        REQUIRE(boost::filesystem::is_regular_file("./xp3-v2~.xp3/abc2.txt"));
        boost::filesystem::remove("./xp3-v2~.xp3/abc2.txt");
        boost::filesystem::remove("./xp3-v2~.xp3/abc.txt");
        boost::filesystem::remove("./xp3-v2~.xp3");
    });
}
