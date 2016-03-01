#include "dec/yuka_script/ykg_image_decoder.h"
#include "algo/range.h"
#include "dec/png/png_image_decoder.h"
#include "err.h"

using namespace au;
using namespace au::dec::yuka_script;

namespace
{
    struct Header final
    {
        bool encrypted;
        uoff_t data_offset; size_t data_size;
        uoff_t regions_offset; size_t regions_size;
    };

    struct Region final
    {
        size_t width, height;
        size_t x, y;
    };
}

static const bstr magic = "YKG000"_b;

static std::unique_ptr<Header> read_header(io::BaseByteStream &input_stream)
{
    auto header = std::make_unique<Header>();
    header->encrypted = input_stream.read_le<u16>() > 0;

    const auto header_size = input_stream.read_le<u32>();
    if (header_size != 64)
        throw err::NotSupportedError("Unexpected header size");
    input_stream.skip(28);

    header->data_offset = input_stream.read_le<u32>();
    header->data_size = input_stream.read_le<u32>();
    input_stream.skip(8);

    header->regions_offset = input_stream.read_le<u32>();
    header->regions_size = input_stream.read_le<u32>();
    return header;
}

static std::vector<std::unique_ptr<Region>> read_regions(
    io::BaseByteStream &input_stream, Header &header)
{
    std::vector<std::unique_ptr<Region>> regions;

    input_stream.seek(header.regions_offset);
    const auto region_count = header.regions_size / 64;
    for (const auto i : algo::range(region_count))
    {
        auto region = std::make_unique<Region>();
        region->x = input_stream.read_le<u32>();
        region->y = input_stream.read_le<u32>();
        region->width = input_stream.read_le<u32>();
        region->height = input_stream.read_le<u32>();
        input_stream.skip(48);
        regions.push_back(std::move(region));
    }
    return regions;
}

static res::Image decode_png(
    const Logger &logger, io::File &input_file, Header &header)
{
    input_file.stream.seek(header.data_offset);
    bstr data = input_file.stream.read(header.data_size);
    if (data.empty())
        throw std::logic_error("File doesn't contain any data");
    if (data.substr(1, 3) != "GNP"_b)
    {
        throw err::NotSupportedError(
            "Decoding non-PNG based YKG images is not supported");
    }
    data[1] = 'P';
    data[2] = 'N';
    data[3] = 'G';

    io::File png_file(input_file.path, data);
    const auto png_image_decoder = dec::png::PngImageDecoder();
    return png_image_decoder.decode(logger, png_file);
}

bool YkgImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image YkgImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.skip(magic.size());

    auto header = read_header(input_file.stream);
    if (header->encrypted)
    {
        throw err::NotSupportedError(
            "Decoding encrypted YKG images is not supported");
    }

    read_regions(input_file.stream, *header);
    return decode_png(logger, input_file, *header);
}

static auto _ = dec::register_decoder<YkgImageDecoder>("yuka-script/ykg");
