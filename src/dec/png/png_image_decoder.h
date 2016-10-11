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

#include "dec/base_image_decoder.h"

namespace au {
namespace dec {
namespace png {

    class PngImageDecoder final : public BaseImageDecoder
    {
    public:
        using ChunkHandler = std::function<void(
            const std::string &chunk_name, const bstr &chunk_data)>;

        using BaseImageDecoder::decode;
        res::Image decode(
            const Logger &logger,
            io::File &input_file,
            ChunkHandler chunk_handler) const;

    protected:
        bool is_recognized_impl(io::File &input_file) const override;
        res::Image decode_impl(
            const Logger &logger, io::File &input_file) const override;
    };

} } }
