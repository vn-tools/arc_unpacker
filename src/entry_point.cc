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

#include "entry_point.h"
#include <boost/filesystem.hpp>
#include <boost/filesystem/detail/utf8_codecvt_facet.hpp>
#include <boost/locale.hpp>
#include "algo/locale.h"

using namespace au;

std::vector<std::string> get_arguments(int argc, const char **argv)
{
    std::vector<std::string> arguments;
    for (int i = 0; i < argc; i++)
        arguments.push_back(std::string(argv[i]));
    return arguments;
}

std::vector<std::string> get_arguments(int argc, const wchar_t **argv)
{
    std::vector<std::string> arguments;
    for (int i = 0; i < argc; i++)
    {
        const auto arg = algo::utf16_to_utf8(
            bstr(reinterpret_cast<const char*>(argv[i]), wcslen(argv[i]) * 2));
        arguments.push_back(arg.str());
    }
    return arguments;
}

void init_fs_utf8()
{
    std::locale loc(
        std::locale(),
        new boost::filesystem::detail::utf8_codecvt_facet);
    boost::filesystem::path::imbue(loc);
}
