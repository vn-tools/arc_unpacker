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

#define CATCH_CONFIG_RUNNER
#include "entry_point.h"
#include "io/program_path.h"
#include "test_support/catch.h"
#include "virtual_file_system.h"

using namespace au;

struct Listener final : Catch::TestEventListenerBase
{
    using TestEventListenerBase::TestEventListenerBase;

    void testCaseStarting(Catch::TestCaseInfo const &test_info) override
    {
        VirtualFileSystem::clear();
    }
};

CATCH_REGISTER_LISTENER(Listener)

int main(int argc, char *argv[])
{
    io::set_program_path_from_arg(argv[0]);
    init_fs_utf8();
    return Catch::Session().run(argc, argv);
}
