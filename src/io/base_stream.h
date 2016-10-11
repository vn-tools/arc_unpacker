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
#include "types.h"

namespace au {
namespace io {

    class BaseStream
    {
    public:
        virtual ~BaseStream() = 0;

        uoff_t left() const;
        virtual uoff_t size() const = 0;
        virtual uoff_t pos() const = 0;
        virtual BaseStream &seek(const uoff_t offset) = 0;
        virtual BaseStream &resize(const uoff_t new_size) = 0;

        // virtual allows changing return type in method chaining
        virtual BaseStream &skip(const soff_t offset);
        virtual BaseStream &peek(
            const uoff_t offset, const std::function<void()> &func);
    };

} }
