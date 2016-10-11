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

#include <functional>
#include <memory>
#include "io/file.h"
#include "io/path.h"

namespace au {

    class VirtualFileSystem final
    {
    public:
        static void enable();
        static void disable();

        static void clear();
        static void register_file(
            const io::path &path,
            const std::function<std::unique_ptr<io::File>()> factory);
        static void unregister_file(const io::path &path);

        static void register_directory(const io::path &path);
        static void unregister_directory(const io::path &path);

        static std::unique_ptr<io::File> get_by_stem(const std::string &stem);
        static std::unique_ptr<io::File> get_by_name(const std::string &name);
        static std::unique_ptr<io::File> get_by_path(const io::path &path);
    };

}
