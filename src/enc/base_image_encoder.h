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

#include "io/file.h"
#include "logger.h"
#include "res/image.h"

namespace au {
namespace enc {

    class BaseImageEncoder
    {
    public:
        virtual ~BaseImageEncoder() {}

        std::unique_ptr<io::File> encode(
            const Logger &logger,
            const res::Image &input_image,
            const io::path &name) const;

    protected:
        virtual void encode_impl(
            const Logger &logger,
            const res::Image &input_image,
            io::File &output_file) const = 0;
    };

} }
