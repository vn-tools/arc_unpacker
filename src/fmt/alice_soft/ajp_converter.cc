// AJP JPEG image wrapper
//
// Company:   Alice Soft
// Engine:    -
// Extension: .ajp
// Archives:  AFA
//
// Known games:
// - Daiakuji

#include "fmt/alice_soft/ajp_converter.h"
#include "fmt/alice_soft/pm_converter.h"
#include "io/buffered_io.h"
#include "util/colors.h"
#include "util/image.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt;
using namespace au::fmt::alice_soft;

static const std::string magic = "AJP\x00"_s;
static const std::string key =
    "\x5d\x91\xAE\x87\x4A\x56\x41\xCD\x83\xEC\x4C\x92\xB5\xCB\x16\x34";

static std::unique_ptr<io::IO> decrypt(io::IO &input_io, size_t size)
{
    std::unique_ptr<io::BufferedIO> output_io(new io::BufferedIO);
    output_io->reserve(size);
    for (auto i : util::range(key.size()))
        if (output_io->tell() < size)
            output_io->write_u8(input_io.read_u8() ^ key[i]);
    output_io->write_from_io(input_io, size - output_io->tell());
    output_io->seek(0);
    return std::unique_ptr<io::IO>(std::move(output_io));
}

bool AjpConverter::is_recognized_internal(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> AjpConverter::decode_internal(File &file) const
{
    file.io.skip(magic.size());
    file.io.skip(4 * 2);
    auto width = file.io.read_u32_le();
    auto height = file.io.read_u32_le();
    auto jpeg_offset = file.io.read_u32_le();
    auto jpeg_size = file.io.read_u32_le();
    auto mask_offset = file.io.read_u32_le();
    auto mask_size = file.io.read_u32_le();

    file.io.seek(jpeg_offset);
    auto jpeg_io = decrypt(file.io, jpeg_size);

    file.io.seek(mask_offset);
    auto mask_io = decrypt(file.io, mask_size);

    if (!mask_size)
    {
        std::unique_ptr<File> output_file(new File);
        output_file->name = file.name;
        output_file->io.write_from_io(*jpeg_io);
        output_file->guess_extension();
        return output_file;
    }

    PmConverter pm_converter;
    auto mask_image = pm_converter.decode_to_image(*mask_io);
    auto jpeg_image = util::Image::from_boxed(*jpeg_io);

    std::unique_ptr<u32[]> pixels(new u32[width * height]);
    for (auto y : util::range(height))
    {
        for (auto x : util::range(width))
        {
            auto color = jpeg_image->color_at(x, y);
            auto alpha = util::color::get_channel(
                mask_image->color_at(x, y), util::color::Channel::Red);
            util::color::set_alpha(color, alpha);
            pixels[y * width + x] = color;
        }
    }

    auto output_image = util::Image::from_pixels(
        width, height,
        std::string(reinterpret_cast<char*>(pixels.get()), width * height * 4),
        util::PixelFormat::RGBA);
    return output_image->create_file(file.name);
}
