#include <boost/filesystem.hpp>
#include "arc_unpacker.h"
#include "log.h"
#include "test_support/catch.hh"

using namespace au;

TEST_CASE("Converting single files with CLI facade", "[core][frontend]")
{
    const ArcUnpacker arc_unpacker(
        {
            "./tests/fmt/real_live/files/g00/AYU_03.g00",
            "--fmt=real-live/g00"
        },
        "0.0");

    Log.mute();
    arc_unpacker.run();
    Log.unmute();

    REQUIRE(boost::filesystem::is_regular_file("./AYU_03.png"));
    boost::filesystem::remove("./AYU_03.png");
}

TEST_CASE("Unpacking archives with CLI facade", "[core][frontend]")
{
    const ArcUnpacker arc_unpacker(
        {
            "./tests/fmt/kirikiri/files/xp3/xp3-v2.xp3",
            "--fmt=kirikiri/xp3",
            "--plugin=noop"
        },
        "0.0");

    Log.mute();
    arc_unpacker.run();
    Log.unmute();

    REQUIRE(boost::filesystem::is_directory("./xp3-v2~.xp3"));
    REQUIRE(boost::filesystem::is_regular_file("./xp3-v2~.xp3/abc.txt"));
    REQUIRE(boost::filesystem::is_regular_file("./xp3-v2~.xp3/abc2.txt"));
    boost::filesystem::remove("./xp3-v2~.xp3/abc2.txt");
    boost::filesystem::remove("./xp3-v2~.xp3/abc.txt");
    boost::filesystem::remove("./xp3-v2~.xp3");
}
