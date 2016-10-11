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

#include <map>
#include "algo/any.h"
#include "types.h"

namespace au {
namespace dec {
namespace borland {

    struct Tpf0Structure final
    {
        std::string type;
        std::string name;
        std::map<std::string, algo::any> properties;
        std::vector<std::unique_ptr<Tpf0Structure>> children;

        template<typename T> const T property(const std::string &key) const
        {
            return properties.at(key).get<T>();
        }
    };

    class Tpf0Decoder final
    {
    public:
        std::unique_ptr<Tpf0Structure> decode(const bstr &input) const;
    };

} } }
