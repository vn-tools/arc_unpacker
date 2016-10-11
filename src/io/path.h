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

namespace au {
namespace io {

    class path final
    {
    public:
        path();
        path(const char *s);
        path(const std::string &s);

        const char *c_str() const;
        std::string str() const;
        std::wstring wstr() const;

        path make_relative(const path &base) const;
        path parent() const;
        std::string name() const;
        std::string stem() const;
        std::string extension() const;

        bool is_absolute() const;
        bool is_root() const;
        bool has_extension() const;
        bool has_extension(const std::string &extension) const;
        io::path &change_extension(const std::string &new_extension);
        io::path &change_stem(const std::string &new_extension);

        bool operator ==(const path &other) const;
        bool operator <(const path &other) const;

        path operator /(const path &other) const;
        void operator /=(const path &other);

    private:
        std::string p;
    };

} }
