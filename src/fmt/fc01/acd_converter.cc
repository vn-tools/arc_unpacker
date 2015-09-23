#include "fmt/fc01/acd_converter.h"
#include "fmt/fc01/custom_lzss.h"
#include "err.h"
#include "io/bit_reader.h"
#include "util/image.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::fc01;

static const bstr magic = "ACD 1.00"_b;

bool AcdConverter::is_recognized_internal(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

static bstr do_decode(const bstr &input, size_t canvas_size)
{
    io::BitReader bit_reader(input);
    bstr output(canvas_size);
    auto output_ptr = output.get<u8>();
    auto output_end = output.end<const u8>();
    while (output_ptr < output_end)
    {
        s32 byte = 0;
        if (bit_reader.get(1))
        {
            byte--;
            if (!bit_reader.get(1))
            {
                byte += 3;

                int bit = 0;
                while (!bit)
                {
                    bit = bit_reader.get(1);
                    byte = (byte << 1) | bit;
                    bit = (byte >> 8) & 1;
                    byte &= 0xFF;
                }

                if (byte)
                {
                    byte++;
                    byte *= 0x28CCCCD;
                    byte >>= 24;
                }
            }
        }
        *output_ptr++ = byte;
    }
    return output;
}

std::unique_ptr<File> AcdConverter::decode_internal(File &file) const
{
    file.io.skip(magic.size());
    auto data_offset = file.io.read_u32_le();
    auto size_comp = file.io.read_u32_le();
    auto size_orig = file.io.read_u32_le();
    auto width = file.io.read_u32_le();
    auto height = file.io.read_u32_le();

    file.io.seek(data_offset);
    auto pixel_data = file.io.read(size_comp);
    pixel_data = custom_lzss_decompress(pixel_data, size_orig);
    pixel_data = do_decode(pixel_data, width * height);

    pix::Grid pixels(width, height, pixel_data, pix::Format::Gray8);
    return util::Image::from_pixels(pixels)->create_file(file.name);
}

static auto dummy = fmt::Registry::add<AcdConverter>("fc01/acd");
