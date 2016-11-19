// Copyright (C) 2016 by rr-
//
// This file is part of arc_unpacker.
//
// arc_unpacker is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// arc_unpacker is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with arc_unpacker. If not, see <http://www.gnu.org/licenses/>.

#include "flow/cli_facade.h"
#include "io/file_system.h"
#include "test_support/catch.h"

using namespace au;

TEST_CASE("CLI facade", "[core]")
{
    Logger logger;
    logger.mute();

    SECTION("Converting single files with CLI facade")
    {
        const flow::CliFacade cli_facade(
            logger,
            {
                "./tests/dec/real_live/files/g00/AYU_03.g00",
                "--dec=real-live/g00"
            });

        cli_facade.run();

        REQUIRE(io::is_regular_file("./AYU_03.png"));
        io::remove("./AYU_03.png");
    }

    SECTION("Unpacking archives with CLI facade")
    {
        const flow::CliFacade cli_facade(
            logger,
            {
                "./tests/dec/kirikiri/files/xp3/xp3-v2.xp3",
                "--dec=kirikiri/xp3",
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
