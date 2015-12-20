#include "fmt/crowd/cwp_image_decoder.h"
#include "fmt/png/png_image_decoder.h"
#include "io/memory_stream.h"

using namespace au;
using namespace au::fmt::crowd;

static const auto magic = "CWDP"_b;

bool CwpImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

res::Image CwpImageDecoder::decode_impl(io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto ihdr = input_file.stream.read(13);
    const auto ihdr_crc = input_file.stream.read_u32_be();
    const auto first_idat_size = input_file.stream.read_u32_be();
    const auto rest = input_file.stream.read(
        input_file.stream.size() - 1 - input_file.stream.tell());

    io::File png_file("dummy.png", ""_b);
    png_file.stream.write("\x89""PNG"_b);
    png_file.stream.write("\x0D\x0A\x1A\x0A"_b);
    png_file.stream.write_u32_be(ihdr.size());
    png_file.stream.write("IHDR"_b);
    png_file.stream.write(ihdr);
    png_file.stream.write_u32_be(ihdr_crc);

    png_file.stream.write_u32_be(first_idat_size);
    png_file.stream.write("IDAT"_b);
    png_file.stream.write(rest);

    png_file.stream.write_u32_be(0);
    png_file.stream.write("IEND"_b);
    png_file.stream.write("\xAE\x42\x60\x82"_b);

    const fmt::png::PngImageDecoder png_decoder;
    auto image = png_decoder.decode(png_file);
    for (auto &c : image)
    {
        std::swap(c.b, c.r);
        c.a = 0xFF;
    }
    return image;
}

static auto dummy = fmt::register_fmt<CwpImageDecoder>("crowd/cwp");
