#include <boost/filesystem.hpp>
#include "arc_unpacker.h"
#include "test_support/catch.hh"
#include "test_support/suppress_output.h"

using namespace au;

TEST_CASE("Converting single files with CLI facade works")
{
    suppress_output([&]()
    {
        ArcUnpacker arc_unpacker(
            {
                "arc_unpacker_executable",
                "./tests/fmt/real_live/files/g00-2/AYU_03.g00",
                "--fmt=rl/g00"
            },
            "0.0");

        arc_unpacker.run();

        REQUIRE(boost::filesystem::is_regular_file("./AYU_03.png"));
        boost::filesystem::remove("./AYU_03.png");
    });
}

TEST_CASE("Unpacking archives with CLI facade works")
{
    suppress_output([&]()
    {
        ArcUnpacker arc_unpacker(
            {
                "arc_unpacker_executable",
                "./tests/fmt/kirikiri/files/xp3/xp3-v2.xp3",
                "--fmt=krkr/xp3",
                "--plugin=noop"
            },
            "0.0");

        arc_unpacker.run();

        REQUIRE(boost::filesystem::is_directory("./xp3-v2~.xp3"));
        REQUIRE(boost::filesystem::is_regular_file("./xp3-v2~.xp3/abc.txt"));
        REQUIRE(boost::filesystem::is_regular_file("./xp3-v2~.xp3/abc2.txt"));
        boost::filesystem::remove("./xp3-v2~.xp3/abc2.txt");
        boost::filesystem::remove("./xp3-v2~.xp3/abc.txt");
        boost::filesystem::remove("./xp3-v2~.xp3");
    });
}
