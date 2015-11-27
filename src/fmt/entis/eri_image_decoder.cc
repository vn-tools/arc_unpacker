#include "fmt/entis/eri_image_decoder.h"
#include "err.h"
#include "fmt/entis/common/enums.h"
#include "fmt/entis/common/gamma_decoder.h"
#include "fmt/entis/common/huffman_decoder.h"
#include "fmt/entis/common/nemesis_decoder.h"
#include "fmt/entis/common/sections.h"
#include "fmt/entis/image/lossless.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::entis;

static const bstr magic1 = "Entis\x1A\x00\x00"_b;
static const bstr magic2 = "\x00\x01\x00\x03\x00\x00\x00\x00"_b;
static const bstr magic3 = "Entis Rasterized Image"_b;

static image::EriHeader read_header(
    io::Stream &stream, common::SectionReader &section_reader)
{
    auto header_section = section_reader.get_section("Header");
    stream.seek(header_section.offset);
    common::SectionReader header_section_reader(stream);
    header_section = header_section_reader.get_section("ImageInf");
    stream.seek(header_section.offset);

    image::EriHeader header;
    header.version = stream.read_u32_le();
    header.transformation
        = static_cast<common::Transformation>(stream.read_u32_le());
    header.architecture
        = static_cast<common::Architecture>(stream.read_u32_le());

    header.format_type      = stream.read_u32_le();
    s32 width               = stream.read_u32_le();
    s32 height              = stream.read_u32_le();
    header.width            = std::abs(width);
    header.height           = std::abs(height);
    header.flip             = height > 0;
    header.bit_depth        = stream.read_u32_le();
    header.clipped_pixel    = stream.read_u32_le();
    header.sampling_flags   = stream.read_u32_le();
    header.quantumized_bits = stream.read_u64_le();
    header.allotted_bits    = stream.read_u64_le();
    header.blocking_degree  = stream.read_u32_le();
    header.lapped_block     = stream.read_u32_le();
    header.frame_transform  = stream.read_u32_le();
    header.frame_degree     = stream.read_u32_le();
    return header;
}

bool EriImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic1.size()) == magic1
        && input_file.stream.read(magic2.size()) == magic2
        && input_file.stream.read(magic3.size()) == magic3;
}

static bstr decode_pixel_data(
    const image::EriHeader &header, const bstr &encoded_pixel_data)
{
    std::unique_ptr<common::Decoder> decoder;
    if (header.architecture == common::Architecture::RunLengthGamma)
        decoder.reset(new common::GammaDecoder());
    else if (header.architecture == common::Architecture::RunLengthHuffman)
        decoder.reset(new common::HuffmanDecoder());
    else if (header.architecture == common::Architecture::Nemesis)
        decoder.reset(new common::NemesisDecoder());
    else
    {
        throw err::NotSupportedError(util::format(
            "Architecture type %d not supported", header.architecture));
    }

    if (header.transformation != common::Transformation::Lossless)
    {
        throw err::NotSupportedError(util::format(
            "Transformation type %d not supported", header.transformation));
    }

    decoder->set_input(encoded_pixel_data);
    return decode_lossless_pixel_data(header, *decoder);
}

pix::Image EriImageDecoder::decode_impl(io::File &input_file) const
{
    input_file.stream.seek(0x40);

    common::SectionReader section_reader(input_file.stream);
    auto header = read_header(input_file.stream, section_reader);
    if (header.version != 0x00020100 && header.version != 0x00020200)
        throw err::UnsupportedVersionError(header.version);

    auto stream_section = section_reader.get_section("Stream");
    input_file.stream.seek(stream_section.offset);
    common::SectionReader stream_section_reader(input_file.stream);

    auto pixel_data_sections = stream_section_reader.get_sections("ImageFrm");
    if (!pixel_data_sections.size())
        throw err::CorruptDataError("No pixel data found");

    pix::Image image(header.width, header.height * pixel_data_sections.size());

    for (const auto i : util::range(pixel_data_sections.size()))
    {
        const auto &pixel_data_section = pixel_data_sections[i];
        input_file.stream.seek(pixel_data_section.offset);
        const auto pixel_data = decode_pixel_data(
            header, input_file.stream.read(pixel_data_section.size));

        pix::PixelFormat fmt;
        if (header.bit_depth == 32)
            fmt = pix::PixelFormat::BGRA8888;
        else if (header.bit_depth == 24)
            fmt = pix::PixelFormat::BGR888;
        else if (header.bit_depth == 8)
            fmt = pix::PixelFormat::Gray8;
        else
            throw err::UnsupportedBitDepthError(header.bit_depth);

        pix::Image subimage(
            header.width, header.height, pixel_data, fmt);
        if (header.flip)
            subimage.flip_vertically();
        image.paste(subimage, 0, i * header.height);
    }

    return image;
}

static auto dummy = fmt::register_fmt<EriImageDecoder>("entis/eri");
