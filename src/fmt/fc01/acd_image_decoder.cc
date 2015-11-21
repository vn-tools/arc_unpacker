#include "fmt/fc01/acd_image_decoder.h"
#include "err.h"
#include "fmt/fc01/common/custom_lzss.h"
#include "io/bit_reader.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::fc01;

static const bstr magic = "ACD 1.00"_b;

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

bool AcdImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

pix::Grid AcdImageDecoder::decode_impl(io::File &input_file) const
{
    input_file.stream.skip(magic.size());
    auto data_offset = input_file.stream.read_u32_le();
    auto size_comp = input_file.stream.read_u32_le();
    auto size_orig = input_file.stream.read_u32_le();
    auto width = input_file.stream.read_u32_le();
    auto height = input_file.stream.read_u32_le();

    input_file.stream.seek(data_offset);
    auto pixel_data = input_file.stream.read(size_comp);
    pixel_data = common::custom_lzss_decompress(pixel_data, size_orig);
    pixel_data = do_decode(pixel_data, width * height);

    return pix::Grid(width, height, pixel_data, pix::Format::Gray8);
}

static auto dummy = fmt::register_fmt<AcdImageDecoder>("fc01/acd");
