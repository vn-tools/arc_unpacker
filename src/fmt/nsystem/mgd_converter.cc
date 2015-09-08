// MGD image
//
// Company:   -
// Engine:    NSystem
// Extension: .MGD
// Archives:  FJSYS
//
// Known games:
// - [Fuguriya] [060929] Sonohana 1
// - [Fuguriya] [070529] Sonohana 2 - Watashi no Ouji-sama
// - [Fuguriya] [070714] Sonohana 3 - Anata to Koibito Tsunagi
// - [Fuguriya] [080606] Sonohana 4 - Itoshisa no Photograph
// - [Fuguriya] [080725] Sonohana 5 - Anata o Suki na Shiawase
// - [Fuguriya] [080725] Sonohana 6 - Kuchibiru to Kiss de Tsubuyaite
// - [Fuguriya] [081205] Sonohana 7 - Amakute Hoshikute Torokeru Chuu
// - [Fuguriya] [091211] Sonohana 8 - Tenshi no Hanabira Zome
// - [Fuguriya] [100212] Sonohana 9 - Amakute Otona no Torokeru Chuu
// - [Fuguriya] [101129] Sonohana 10 - Lily Platinum
// - [Yurin Yurin] [111120] Sonohana 11 - Michael no Otome-tachi
// - [Yurin Yurin] [101008] Hanahira!

#include "fmt/nsystem/mgd_converter.h"
#include "io/buffered_io.h"
#include "util/image.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::nsystem;

namespace
{
    enum class CompressionType : u8
    {
        None = 0,
        Sgd = 1,
        Png = 2,
    };

    struct Region
    {
        u16 x;
        u16 y;
        u16 width;
        u16 height;
    };
}

static const bstr magic = "MGD "_b;

static void decompress_sgd_alpha(const bstr &input, io::IO &output_io)
{
    io::BufferedIO input_io(input);
    while (!input_io.eof())
    {
        auto flag = input_io.read_u16_le();
        if (flag & 0x8000)
        {
            size_t size = (flag & 0x7FFF) + 1;
            u8 alpha = input_io.read_u8();
            for (auto i : util::range(size))
            {
                output_io.skip(3);
                output_io.write_u8(alpha ^ 0xFF);
            }
        }
        else
        {
            while (flag-- && !input_io.eof())
            {
                u8 alpha = input_io.read_u8();
                output_io.skip(3);
                output_io.write_u8(alpha ^ 0xFF);
            }
        }
    }
    output_io.seek(0);
}

static void decompress_sgd_bgr_strategy_1(
    io::IO &input_io, io::IO &output_io, u8 flag)
{
    auto size = flag & 0x3F;
    output_io.skip(-4);
    u8 b = output_io.read_u8();
    u8 g = output_io.read_u8();
    u8 r = output_io.read_u8();
    output_io.skip(1);
    for (auto i : util::range(size))
    {
        u16 delta = input_io.read_u16_le();
        if (delta & 0x8000)
        {
            b += delta & 0x1F;
            g += (delta >> 5) & 0x1F;
            r += (delta >> 10) & 0x1F;
        }
        else
        {
            b += ( delta        & 0xF) * (delta &   0x10 ? -1 : 1);
            g += ((delta >>  5) & 0xF) * (delta &  0x200 ? -1 : 1);
            r += ((delta >> 10) & 0xF) * (delta & 0x4000 ? -1 : 1);
        }

        output_io.write_u8(b);
        output_io.write_u8(g);
        output_io.write_u8(r);
        output_io.skip(1);
    }
}

static void decompress_sgd_bgr_strategy_2(
    io::IO &input_io, io::IO &output_io, u8 flag)
{
    auto size = (flag & 0x3F) + 1;
    u8 b = input_io.read_u8();
    u8 g = input_io.read_u8();
    u8 r = input_io.read_u8();
    for (auto i : util::range(size))
    {
        output_io.write_u8(b);
        output_io.write_u8(g);
        output_io.write_u8(r);
        output_io.skip(1);
    }
}

static void decompress_sgd_bgr_strategy_3(
    io::IO &input_io, io::IO &output_io, u8 flag)
{
    auto size = flag;
    for (auto i : util::range(size))
    {
        output_io.write(input_io.read(3));
        output_io.skip(1);
    }
}

static void decompress_sgd_bgr(const bstr &input, io::IO &output_io)
{
    io::BufferedIO input_io(input);
    while (!input_io.eof())
    {
        u8 flag = input_io.read_u8();
        switch (flag & 0xC0)
        {
            case 0x80:
                decompress_sgd_bgr_strategy_1(input_io, output_io, flag);
                break;

            case 0x40:
                decompress_sgd_bgr_strategy_2(input_io, output_io, flag);
                break;

            case 0:
                decompress_sgd_bgr_strategy_3(input_io, output_io, flag);
                break;

            default:
                throw std::runtime_error("Bad decompression flag");
        }
    }
    output_io.seek(0);
}

static bstr decompress_sgd(const bstr &input, size_t output_size)
{
    bstr output;
    output.resize(output_size);
    io::BufferedIO output_io(output);

    io::BufferedIO tmp_io(input);

    auto alpha_size = tmp_io.read_u32_le();
    auto alpha_data = tmp_io.read(alpha_size);
    decompress_sgd_alpha(alpha_data, output_io);

    auto color_size = tmp_io.read_u32_le();
    auto color_data = tmp_io.read(color_size);
    decompress_sgd_bgr(color_data, output_io);

    return output_io.read_to_eof();
}

static std::vector<std::unique_ptr<Region>> read_region_data(io::IO &file_io)
{
    std::vector<std::unique_ptr<Region>> regions;
    while (file_io.tell() < file_io.size())
    {
        file_io.skip(4);
        size_t regions_size = file_io.read_u32_le();
        size_t region_count = file_io.read_u16_le();
        size_t meta_format = file_io.read_u16_le();
        size_t bytes_left = file_io.size() - file_io.tell();
        if (meta_format != 4)
            throw std::runtime_error("Unexpected meta format");
        if (regions_size != bytes_left)
            throw std::runtime_error("Unexpected regions size");

        for (auto i : util::range(region_count))
        {
            std::unique_ptr<Region> region(new Region);
            region->x = file_io.read_u16_le();
            region->y = file_io.read_u16_le();
            region->width = file_io.read_u16_le();
            region->height = file_io.read_u16_le();
            regions.push_back(std::move(region));
        }

        if (file_io.tell() + 4 >= file_io.size())
            break;
        file_io.skip(4);
    }
    return regions;
}

static std::unique_ptr<util::Image> read_image(
    const bstr &input,
    CompressionType compression_type,
    size_t size_original,
    size_t width,
    size_t height)
{
    if (compression_type == CompressionType::None)
    {
        pix::Grid pixels(width, height, input, pix::Format::BGRA8888);
        return util::Image::from_pixels(pixels);
    }
    else if (compression_type == CompressionType::Sgd)
    {
        auto decompressed = decompress_sgd(input, size_original);
        pix::Grid pixels(width, height, decompressed, pix::Format::BGRA8888);
        return util::Image::from_pixels(pixels);
    }
    else if (compression_type == CompressionType::Png)
        return util::Image::from_boxed(input);

    throw std::runtime_error("Unsupported compression type");
}

bool MgdConverter::is_recognized_internal(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> MgdConverter::decode_internal(File &file) const
{
    file.io.skip(magic.size());

    u16 data_offset = file.io.read_u16_le();
    u16 format = file.io.read_u16_le();
    file.io.skip(4);
    u16 width = file.io.read_u16_le();
    u16 height = file.io.read_u16_le();
    u32 size_original = file.io.read_u32_le();
    u32 size_compressed_total = file.io.read_u32_le();
    auto compression_type = static_cast<CompressionType>(file.io.read_u32_le());
    file.io.skip(64);

    size_t size_compressed = file.io.read_u32_le();
    if (size_compressed_total != size_compressed + 4)
        throw std::runtime_error("Unexpected compressed size");

    auto image = read_image(
        file.io.read(size_compressed),
        compression_type,
        size_original,
        width,
        height);

    read_region_data(file.io);
    return image->create_file(file.name);
}

static auto dummy = fmt::Registry::add<MgdConverter>("nsystem/mgd");
