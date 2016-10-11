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

#pragma once

#include <string>
#include <vector>

std::vector<std::string> get_arguments(int argc, const char **arg);
std::vector<std::string> get_arguments(int argc, const wchar_t **argv);
void init_fs_utf8();

#ifdef _WIN32
    #define ENTRY_POINT(x) int wmain(int argc, const wchar_t **argv) \
    { \
        std::vector<std::string> arguments = get_arguments(argc, argv); \
        init_fs_utf8(); \
        x \
    }
#else
    #define ENTRY_POINT(x) int main(int argc, const char **argv) \
    { \
        std::vector<std::string> arguments = get_arguments(argc, argv); \
        init_fs_utf8(); \
        x \
    }
#endif
