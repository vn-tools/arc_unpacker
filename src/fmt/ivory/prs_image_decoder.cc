#include "fmt/ivory/prs_image_decoder.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::fmt::ivory;

static const bstr magic = "YB"_b;

static bstr decode_pixels(const bstr &source, size_t width, size_t height)
{
    bstr target(width * height * 3);
    u8 *target_ptr = target.get<u8>();
    u8 *target_end = target_ptr + target.size();
    const u8 *source_ptr = source.get<const u8>();
    const u8 *source_end = source_ptr + source.size();

    int flag = 0;
    int size_lookup[256];
    for (auto i : algo::range(256))
        size_lookup[i] = i + 3;
    size_lookup[0xFF] = 0x1000;
    size_lookup[0xFE] = 0x400;
    size_lookup[0xFD] = 0x100;

    while (source_ptr < source_end && target_ptr < target_end)
    {
        flag <<= 1;
        if ((flag & 0xFF) == 0)
        {
            flag = *source_ptr++;
            flag <<= 1;
            flag += 1;
        }

        if ((flag & 0x100) != 0x100)
        {
            *target_ptr++ = *source_ptr++;
        }
        else
        {
            int tmp = *source_ptr++;
            size_t size = 0;
            size_t shift = 0;

            if (tmp & 0x80)
            {
                shift = (*source_ptr++) | ((tmp & 0x3F) << 8);
                if (tmp & 0x40)
                {
                    if (source_ptr >= source_end)
                        break;
                    auto index = static_cast<size_t>(*source_ptr++);
                    size = size_lookup[index];
                }
                else
                {
                    size = (shift & 0xF) + 3;
                    shift >>= 4;
                }
            }
            else
            {
                size = tmp >> 2;
                tmp &= 3;
                if (tmp == 3)
                {
                    size += 9;
                    for (auto i : algo::range(size))
                    {
                        if (source_ptr >= source_end
                            || target_ptr >= target_end)
                        {
                            break;
                        }
                        *target_ptr++ = *source_ptr++;
                    }
                    continue;
                }
                shift = size;
                size = tmp + 2;
            }

            shift += 1;
            for (auto i : algo::range(size))
            {
                if (target_ptr >= target_end)
                    break;
                if (target_ptr - shift < target.get<u8>())
                    throw err::BadDataOffsetError();
                *target_ptr = *(target_ptr - shift);
                target_ptr++;
            }
        }
    }
    return target;
}

bool PrsImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image PrsImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.skip(magic.size());

    bool using_differences = input_file.stream.read_u8() > 0;
    auto version = input_file.stream.read_u8();
    if (version != 3)
        throw err::UnsupportedVersionError(version);

    u32 source_size = input_file.stream.read_u32_le();
    input_file.stream.skip(4);
    u16 width = input_file.stream.read_u16_le();
    u16 height = input_file.stream.read_u16_le();

    auto target = input_file.stream.read(source_size);
    target = decode_pixels(target, width, height);
    if (using_differences)
        for (auto i : algo::range(3, target.size()))
            target[i] += target[i - 3];

    return res::Image(width, height, target, res::PixelFormat::BGR888);
}

static auto dummy = fmt::register_fmt<PrsImageDecoder>("ivory/prs");
