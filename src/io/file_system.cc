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

#include "io/file_system.h"
#include <boost/filesystem/path.hpp>

using namespace au;
using namespace au::io;

bool io::exists(const path &p)
{
    return boost::filesystem::exists(p.str());
}

bool io::is_regular_file(const path &p)
{
    return boost::filesystem::is_regular_file(p.str());
}

bool io::is_directory(const path &p)
{
    return boost::filesystem::is_directory(p.str());
}

path io::current_working_directory()
{
    return boost::filesystem::current_path().string();
}

path io::absolute(const path &p)
{
    return boost::filesystem::absolute(p.str()).string();
}

void io::create_directories(const path &p)
{
    const auto bp = boost::filesystem::path(p.str());
    if (!bp.empty() && !boost::filesystem::exists(bp))
        boost::filesystem::create_directories(bp);
}

void io::remove(const path &p)
{
    boost::filesystem::remove(p.str());
}
