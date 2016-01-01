#include "dec/fc01/acd_image_decoder.h"
#include "algo/range.h"
#include "dec/fc01/common/custom_lzss.h"
#include "err.h"
#include "io/msb_bit_reader.h"

using namespace au;
using namespace au::dec::fc01;

static const bstr magic = "ACD 1.00"_b;

static bstr do_decode(const bstr &input, size_t canvas_size)
{
    io::MsbBitReader bit_reader(input);
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

res::Image AcdImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
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

    return res::Image(width, height, pixel_data, res::PixelFormat::Gray8);
}

static auto _ = dec::register_decoder<AcdImageDecoder>("fc01/acd");
