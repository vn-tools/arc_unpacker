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

#include <boost/filesystem.hpp>
#include "io/path.h"

namespace au {
namespace io {

    path current_working_directory();
    bool exists(const path &p);
    bool is_directory(const path &p);
    bool is_regular_file(const path &p);
    path absolute(const path &p);

    void create_directories(const path &p);
    void remove(const path &p);

    template<typename T> class BaseDirectoryRange final
    {
    public:
        struct Iterator final
            : std::iterator<std::random_access_iterator_tag, path, path>
        {
            T it;

            inline Iterator(const T it) : it(it)
            {
            }

            inline path operator *()
            {
                return path(it->path().string());
            }

            inline Iterator operator ++()
            {
                it++;
                return *this;
            }

            inline bool operator !=(Iterator other)
            {
                return it != other.it;
            }
        };

        inline BaseDirectoryRange(const path &path) :
            path_copy(path),
            b(Iterator(T(path_copy.str()))),
            e(Iterator(T()))
        {
        }

        inline Iterator begin()
        {
            return b;
        }

        inline Iterator end()
        {
            return e;
        }

    private:
        path path_copy;
        Iterator b, e;
    };

    using DirectoryRange = BaseDirectoryRange
        <boost::filesystem::directory_iterator>;

    using RecursiveDirectoryRange = BaseDirectoryRange
        <boost::filesystem::recursive_directory_iterator>;

    inline DirectoryRange directory_range(const path &path)
    {
        return DirectoryRange(path);
    }

    inline RecursiveDirectoryRange recursive_directory_range(const path &path)
    {
        return RecursiveDirectoryRange(path);
    }

} }
