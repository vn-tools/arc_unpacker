#include "dec/alice_soft/vsp_image_decoder.h"
#include "algo/range.h"
#include "dec/alice_soft/pms_image_decoder.h"

using namespace au;
using namespace au::dec::alice_soft;

bool VspImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("vsp");
}

static bstr decompress_vsp(
    io::BaseByteStream &input_stream, const size_t width, const size_t height)
{
    if (width % 8 != 0)
        throw std::logic_error("Invalid width");

    bstr output(width * height);

    auto buf1 = std::make_unique<u8[][4]>(height);
    auto buf2 = std::make_unique<u8[][4]>(height);

    auto bp = buf1.get();
    auto bc = buf2.get();
    auto mask = 0;

    for (auto x : algo::range(width / 8))
    {
        for (auto plane : algo::range(4))
        {
            size_t y = 0;
            while (y < height)
            {
                auto c = input_stream.read<u8>();
                switch (c)
                {
                    case 0x00:
                    {
                        auto n = input_stream.read<u8>() + 1;
                        while (n-- && y < height)
                        {
                            bc[y][plane] = bp[y][plane];
                            y++;
                        }
                        break;
                    }

                    case 0x01:
                    {
                        auto n = input_stream.read<u8>() + 1;
                        auto b0 = input_stream.read<u8>();
                        while (n-- && y < height)
                            bc[y++][plane] = b0;
                        break;
                    }

                    case 0x02:
                    {
                        auto n = input_stream.read<u8>() + 1;
                        auto b0 = input_stream.read<u8>();
                        auto b1 = input_stream.read<u8>();
                        while (n-- && y < height)
                        {
                            bc[y++][plane] = b0;
                            if (y >= height) break;
                            bc[y++][plane] = b1;
                        }
                        break;
                    }

                    case 0x03:
                    {
                        auto n = input_stream.read<u8>() + 1;
                        while (n-- && y < height)
                        {
                            bc[y][plane] = bc[y][0] ^ mask;
                            y++;
                        }
                        mask = 0;
                        break;
                    }

                    case 0x04:
                    {
                        auto n = input_stream.read<u8>() + 1;
                        while (n-- && y < height)
                        {
                            bc[y][plane] = bc[y][1] ^ mask;
                            y++;
                        }
                        mask = 0;
                        break;
                    }

                    case 0x05:
                    {
                        auto n = input_stream.read<u8>() + 1;
                        while (n-- && y < height)
                        {
                            bc[y][plane] = bc[y][2] ^ mask;
                            y++;
                        }
                        mask = 0;
                        break;
                    }

                    case 0x06:
                        mask = 0xFF;
                        break;

                    case 0x07:
                        bc[y++][plane] = input_stream.read<u8>();
                        break;

                    default:
                        bc[y++][plane] = c;
                        break;
                }
            }
        }

        for (auto y : algo::range(height))
        {
            int component[4];
            for (auto i : algo::range(4))
                component[i] = (bc[y][i] << i) | (bc[y][i] >> (8 - i));

            auto output_ptr = &output[(y * (width / 8) + x) * 8];
            for (auto j : algo::range(8))
            {
                for (auto i : algo::range(4))
                    *output_ptr |= (component[i] >> (7 - j)) & (1 << i);
                output_ptr++;
            }
        }

        auto tmp = bp;
        bp = bc;
        bc = tmp;
    }

    return output;
}

res::Image VspImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    auto x = input_file.stream.read_le<u16>();
    auto y = input_file.stream.read_le<u16>();
    auto width = input_file.stream.read_le<u16>() - x;
    auto height = input_file.stream.read_le<u16>() - y;
    auto use_pms = input_file.stream.read<u8>() > 0;

    std::unique_ptr<res::Image> image;

    if (use_pms)
    {
        width = ((width + 7) / 8) * 8;

        input_file.stream.seek(0x20);
        res::Palette palette(256, input_file.stream, res::PixelFormat::RGB888);

        input_file.stream.seek(0x320);
        auto pixel_data = PmsImageDecoder::decompress_8bit(
            input_file.stream, width, height);
        image.reset(new res::Image(width, height, pixel_data, palette));
    }
    else
    {
        width *= 8;

        input_file.stream.seek(0x0A);
        res::Palette palette(16);
        for (auto &c : palette)
        {
            c.b = (input_file.stream.read<u8>() & 0x0F) * 0x11;
            c.r = (input_file.stream.read<u8>() & 0x0F) * 0x11;
            c.g = (input_file.stream.read<u8>() & 0x0F) * 0x11;
        }

        input_file.stream.seek(0x3A);
        auto pixel_data = decompress_vsp(input_file.stream, width, height);
        image.reset(new res::Image(width, height, pixel_data, palette));
    }

    return *image;
}

static auto _ = dec::register_decoder<VspImageDecoder>("alice-soft/vsp");
