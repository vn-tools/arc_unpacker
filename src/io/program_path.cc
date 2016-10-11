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

#include "io/program_path.h"
#include "err.h"
#include "io/file_system.h"

using namespace au;

namespace
{
    static io::path program_path;
}

void io::set_program_path_from_arg(const std::string &arg)
{
    program_path = io::absolute(io::path(arg)).str();
}

io::path io::get_program_path()
{
    return program_path;
}

io::path io::get_assets_dir_path()
{
    auto dir = program_path.parent();
    do
    {
        const auto path = dir / "etc";
        if (io::is_directory(path))
            return path.str();
        dir = dir.parent();
    }
    while (!dir.is_root());
    throw err::FileNotFoundError("Can't locate 'etc/' assets directory!");
}
