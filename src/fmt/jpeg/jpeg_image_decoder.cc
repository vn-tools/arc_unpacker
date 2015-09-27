#include "fmt/jpeg/jpeg_image_decoder.h"
#include <jpeglib.h>
#include "err.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::jpeg;

static const bstr magic = "\xFF\xD8\xFF"_b;

bool JpegImageDecoder::is_recognized_internal(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

pix::Grid JpegImageDecoder::decode_internal(File &file) const
{
    bstr source = file.io.read_to_eof();

    jpeg_decompress_struct info;
    jpeg_error_mgr err;
    info.err = jpeg_std_error(&err);
    jpeg_create_decompress(&info);
    jpeg_mem_src(&info, source.get<u8>(), source.size());
    jpeg_read_header(&info, true);
    jpeg_start_decompress(&info);

    auto width = info.output_width;
    auto height = info.output_height;
    auto channels = info.num_components;

    pix::Format format;
    if (channels == 3)
        format = pix::Format::RGB888;
    else if (channels == 4)
        format = pix::Format::RGBA8888;
    else
        throw err::NotSupportedError("Bad pixel format");

    bstr raw_data(width * height * channels);
    for (auto y : util::range(height))
    {
        auto ptr = raw_data.get<u8>() + y * width * channels;
        jpeg_read_scanlines(&info, &ptr, 1);
    }
    jpeg_finish_decompress(&info);

    return pix::Grid(width, height, raw_data, format);
}

static auto dummy = fmt::Registry::add<JpegImageDecoder>("jpeg/jpeg");
