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

#include "dec/google/webp_image_decoder.h"
#include "err.h"
#if WEBP_FOUND
    #include "webp/decode.h"
#endif

using namespace au;
using namespace au::dec::google;

bool WebpImageDecoder::is_recognized_impl(io::File &input_file) const
{
    if (input_file.stream.seek(0).read(4) != "RIFF"_b)
        return false;
    return input_file.stream.seek(8).read(4) == "WEBP"_b;
}

res::Image WebpImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
#if WEBP_FOUND
    const auto input_data = input_file.stream.seek(0).read_to_eof();

    int width = 0, height = 0;
    if (!WebPGetInfo(
            input_data.get<const u8>(), input_data.size(), &width, &height))
    {
        throw err::CorruptDataError("Corrupt WEBP data");
    }

    bstr output_data(width * height * 4);
    if (!WebPDecodeBGRAInto(
            input_data.get<const u8>(),
            input_data.size(),
            output_data.get<u8>(),
            output_data.size(),
            width * 4))
    {
        throw err::CorruptDataError("Corrupt WEBP data");
    }

    res::Image image(
        width,
        height,
        output_data,
        res::PixelFormat::BGRA8888);

    return image;
#else
    throw err::NotSupportedError("webp image decoder is not available.");
#endif
}

static auto _ = dec::register_decoder<WebpImageDecoder>("google/webp");
