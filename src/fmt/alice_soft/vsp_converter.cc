// VSP image
//
// Company:   Alice Soft
// Engine:    AliceSystem / NV-SYSTEM
// Extension: .VSP
// Archives:  ALD
//
// Known games:
// - [Alice Soft] [970404] Rance 4 - Kyoudan no Isan
// - [Alice Soft] [971218] Dalk
// - [Alice Soft] [971218] Dr. Stop!
// - [Alice Soft] [971218] Rance 1 - Hikari o Motomete
// - [Alice Soft] [971218] Rance 2 - Hangyaku no Shoujo-tachi
// - [Alice Soft] [971218] Rance 3 - Leazas Kanraku
// - [Alice Soft] [971218] Toushin Toshi

#include "fmt/alice_soft/vsp_converter.h"
#include "fmt/alice_soft/pms_converter.h"
#include "err.h"
#include "util/image.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::alice_soft;

bool VspConverter::is_recognized_internal(File &file) const
{
    return file.has_extension("vsp");
}

static bstr decompress_vsp(io::IO &input_io, size_t width, size_t height)
{
    if (width % 8 != 0)
        throw std::logic_error("Invalid width");

    bstr output(width * height);

    std::unique_ptr<u8[][4]> buf1(new u8[height][4]);
    std::unique_ptr<u8[][4]> buf2(new u8[height][4]);

    auto bp = buf1.get();
    auto bc = buf2.get();
    auto mask = 0;

    for (auto x : util::range(width / 8))
    {
        for (auto plane : util::range(4))
        {
            size_t y = 0;
            while (y < height)
            {
                auto c = input_io.read_u8();
                switch (c)
                {
                    case 0x00:
                    {
                        auto n = input_io.read_u8() + 1;
                        while (n-- && y < height)
                        {
                            bc[y][plane] = bp[y][plane];
                            y++;
                        }
                        break;
                    }

                    case 0x01:
                    {
                        auto n = input_io.read_u8() + 1;
                        auto b0 = input_io.read_u8();
                        while (n-- && y < height)
                            bc[y++][plane] = b0;
                        break;
                    }

                    case 0x02:
                    {
                        auto n = input_io.read_u8() + 1;
                        auto b0 = input_io.read_u8();
                        auto b1 = input_io.read_u8();
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
                        auto n = input_io.read_u8() + 1;
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
                        auto n = input_io.read_u8() + 1;
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
                        auto n = input_io.read_u8() + 1;
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
                        bc[y++][plane] = input_io.read_u8();
                        break;

                    default:
                        bc[y++][plane] = c;
                        break;
                }
            }
        }

        for (auto y : util::range(height))
        {
            int component[4];
            for (auto i : util::range(4))
                component[i] = (bc[y][i] << i) | (bc[y][i] >> (8 - i));

            auto output_ptr = &output[(y * (width / 8) + x) * 8];
            for (auto j : util::range(8))
            {
                for (auto i : util::range(4))
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

std::unique_ptr<File> VspConverter::decode_internal(File &file) const
{
    auto x = file.io.read_u16_le();
    auto y = file.io.read_u16_le();
    auto width = (file.io.read_u16_le() - x);
    auto height = file.io.read_u16_le() - y;
    auto use_pms = file.io.read_u8() > 0;

    std::unique_ptr<pix::Grid> pixels;

    if (use_pms)
    {
        width = ((width + 7) / 8) * 8;

        file.io.seek(0x20);
        pix::Palette palette(256, file.io, pix::Format::BGR888);

        file.io.seek(0x320);
        auto pixel_data = PmsConverter::decompress_8bit(file.io, width, height);
        pixels.reset(new pix::Grid(width, height, pixel_data, palette));
    }
    else
    {
        width *= 8;

        file.io.seek(0x0A);
        pix::Palette palette(16);
        for (auto &c : palette)
        {
            c.b = (file.io.read_u8() & 0x0F) * 0x11;
            c.r = (file.io.read_u8() & 0x0F) * 0x11;
            c.g = (file.io.read_u8() & 0x0F) * 0x11;
        }

        file.io.seek(0x3A);
        auto pixel_data = decompress_vsp(file.io, width, height);
        pixels.reset(new pix::Grid(width, height, pixel_data, palette));
    }

    return util::Image::from_pixels(*pixels)->create_file(file.name);
}

static auto dummy = fmt::Registry::add<VspConverter>("alice/vsp");
