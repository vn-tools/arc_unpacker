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
