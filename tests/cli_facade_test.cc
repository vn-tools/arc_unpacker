#include "cli_facade.h"
#include "io/file_system.h"
#include "test_support/catch.h"

using namespace au;

TEST_CASE("CLI facade", "[core]")
{
    Logger logger;
    logger.mute();

    SECTION("Converting single files with CLI facade")
    {
        const CliFacade cli_facade(
            logger,
            {
                "./tests/fmt/real_live/files/g00/AYU_03.g00",
                "--fmt=real-live/g00"
            });

        cli_facade.run();

        REQUIRE(io::is_regular_file("./AYU_03.png"));
        io::remove("./AYU_03.png");
    }

    SECTION("Unpacking archives with CLI facade")
    {
        const CliFacade cli_facade(
            logger,
            {
                "./tests/fmt/kirikiri/files/xp3/xp3-v2.xp3",
                "--fmt=kirikiri/xp3",
                "--plugin=noop"
            });

        cli_facade.run();

        REQUIRE(io::is_directory("./xp3-v2~.xp3"));
        REQUIRE(io::is_regular_file("./xp3-v2~.xp3/123.txt"));
        REQUIRE(io::is_regular_file("./xp3-v2~.xp3/abc.xyz"));
        io::remove("./xp3-v2~.xp3/abc.xyz");
        io::remove("./xp3-v2~.xp3/123.txt");
        io::remove("./xp3-v2~.xp3");
    }
}
