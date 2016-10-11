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

#include "dec/jpeg/jpeg_image_decoder.h"
#include <jpeglib.h>
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::jpeg;

static const bstr magic = "\xFF\xD8\xFF"_b;

bool JpegImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image JpegImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    bstr source = input_file.stream.read_to_eof();

    jpeg_decompress_struct info;
    jpeg_error_mgr err;
    info.err = jpeg_std_error(&err);
    jpeg_create_decompress(&info);
    jpeg_mem_src(&info, source.get<u8>(), source.size());
    jpeg_read_header(&info, true);
    jpeg_start_decompress(&info);

    const auto width = info.output_width;
    const auto height = info.output_height;
    const auto channels = info.num_components;

    res::PixelFormat format;
    if (channels == 3)
        format = res::PixelFormat::RGB888;
    else if (channels == 4)
        format = res::PixelFormat::RGBA8888;
    else if (channels == 1)
        format = res::PixelFormat::Gray8;
    else
    {
        jpeg_finish_decompress(&info);
        jpeg_destroy_decompress(&info);
        throw err::UnsupportedChannelCountError(channels);
    }

    bstr raw_data(width * height * channels);
    for (const auto y : algo::range(height))
    {
        auto ptr = raw_data.get<u8>() + y * width * channels;
        jpeg_read_scanlines(&info, &ptr, 1);
    }
    jpeg_finish_decompress(&info);
    jpeg_destroy_decompress(&info);

    return res::Image(width, height, raw_data, format);
}

static auto _ = dec::register_decoder<JpegImageDecoder>("jpeg/jpeg");
