#include "dec/lucifen/elg_image_decoder.h"
#include "algo/ptr.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::lucifen;

static const bstr magic = "ELG"_b;

namespace
{
    struct Header final
    {
        int x, y;
        size_t width, height;
        size_t depth;
        size_t type;
    };
}

static Header read_header(io::BaseByteStream &input_stream)
{
    Header header;

    input_stream.seek(magic.size());
    const auto tmp = input_stream.read<u8>();

    if (tmp == 2)
    {
        header.type = 2;
        header.depth = input_stream.read<u8>();
        header.width = input_stream.read_le<u16>();
        header.height = input_stream.read_le<u16>();
        header.x = input_stream.read_le<s16>();
        header.y = input_stream.read_le<s16>();
        return header;
    }

    if (tmp == 1)
    {
        header.type = 1;
        header.depth = input_stream.read<u8>();
        header.x = input_stream.read_le<s16>();
        header.y = input_stream.read_le<s16>();
        header.width = input_stream.read_le<u16>();
        header.height = input_stream.read_le<u16>();
        return header;
    }

    header.depth = tmp;
    header.type = 0;
    header.width = input_stream.read_le<u16>();
    header.height = input_stream.read_le<u16>();
    return header;
}

static bstr read_rgb(
    const size_t width, const size_t height, io::BaseByteStream &input_stream)
{
    bstr output(width * height * 3);
    auto output_ptr = algo::make_ptr(output);

    while (true)
    {
        const auto flags = input_stream.read<u8>();
        if (flags == 0xFF || !output_ptr.left())
            break;

        const auto method = (flags >> 6) & 3;
        if (method == 0)
        {
            const size_t count = (flags & 0x20) != 0
                ? (((flags & 0x1F) << 8) + input_stream.read<u8>() + 0x21)
                : ((flags & 0x1F) + 1);
            output_ptr.append_from(input_stream.read(count * 3));
        }

        else if (method == 1)
        {
            const size_t count = (flags & 0x20) != 0
                ? (((flags & 0x1F) << 8) + input_stream.read<u8>() + 0x22)
                : ((flags & 0x1F) + 2);
            const auto base = input_stream.read(3);
            for (const auto i : algo::range(count))
                output_ptr.append_from(base);
        }

        else if (method == 2)
        {
            const auto sub_method = (flags >> 4) & 3;
            size_t count, pos;
            if (sub_method == 0)
            {
                count = (flags & 0xF) + 1;
                pos = input_stream.read<u8>() + 2;
            }
            else if (sub_method == 1)
            {
                pos = ((flags & 0xF) << 8) + input_stream.read<u8>() + 2;
                count = input_stream.read<u8>() + 1;
            }
            else if (sub_method == 2)
            {
                const auto b1 = input_stream.read<u8>();
                const auto b2 = input_stream.read<u8>();
                pos = ((((flags & 0xF) << 8) + b1) << 8) + b2 + 0x1002;
                count = input_stream.read<u8>() + 1;
            }
            else
            {
                pos = (flags & 8) != 0
                    ? (((flags & 0x7) << 8) + input_stream.read<u8>() + 0xA)
                    : ((flags & 0x7) + 2);
                count = 1;
            }
            output_ptr.append_from(-pos * 3, count * 3);
        }

        else
        {
            int y, x;

            const auto sub_method = (flags >> 4) & 3;
            if (sub_method == 0)
            {
                const auto sub_method_2 = (flags >> 2) & 3;
                if (sub_method_2 == 0)
                {
                    y = ((flags & 0x3) << 8) + input_stream.read<u8>() + 0x10;
                    x = 0;
                }
                if (sub_method_2 == 1)
                {
                    y = ((flags & 0x3) << 8) + input_stream.read<u8>() + 0x10;
                    x = -1;
                }
                if (sub_method_2 == 2)
                {
                    y = ((flags & 0x3) << 8) + input_stream.read<u8>() + 0x10;
                    x = 1;
                }
                if (sub_method_2 == 3)
                {
                    const auto b = input_stream.read<u8>();
                    const auto pos = ((flags & 0x3) << 8) + b + 0x80A;
                    output_ptr.append_from(-pos * 3, 3);
                    continue;
                }
            }

            else if (sub_method == 1)
            {
                y = (flags & 0xF) + 1;
                x = 0;
            }
            else if (sub_method == 2)
            {
                y = (flags & 0xF) + 1;
                x = -1;
            }
            else
            {
                y = (flags & 0xF) + 1;
                x = 1;
            }

            const auto pos = x - width * y;
            output_ptr.append_from(pos * 3, 3);
        }
    }

    return output;
}

static bstr read_mono(
    const size_t width, const size_t height, io::BaseByteStream &input_stream)
{
    bstr output(width * height);
    auto output_ptr = algo::make_ptr(output);

    while (true)
    {
        const auto flags = input_stream.read<u8>();
        if (flags == 0xFF || !output_ptr.left())
            break;

        const auto method = (flags >> 6) & 3;
        if (method == 0)
        {
            const size_t count = (flags & 0x20) != 0
                ? (((flags & 0x1F) << 8) + input_stream.read<u8>() + 0x21)
                : ((flags & 0x1F) + 1);
            output_ptr.append_from(input_stream.read(count));
        }

        else if (method == 1)
        {
            const size_t count = (flags & 0x20) != 0
                ? (((flags & 0x1F) << 8) + input_stream.read<u8>() + 0x23)
                : ((flags & 0x1F) + 3);
            const auto base = input_stream.read(1);
            for (const auto i : algo::range(count))
                output_ptr.append_from(base);
        }

        else if (method == 2)
        {
            size_t count, pos;
            const auto sub_method = (flags >> 4) & 3;
            if (sub_method == 0)
            {
                count = (flags & 0xF) + 2;
                pos = input_stream.read<u8>() + 2;
            }
            else if (sub_method == 1)
            {
                pos = ((flags & 0xF) << 8) + input_stream.read<u8>() + 3;
                count = input_stream.read<u8>() + 4;
            }
            else if (sub_method == 2)
            {
                pos = ((flags & 0xF) << 8) + input_stream.read<u8>() + 3;
                count = 3;
            }
            else
            {
                pos = ((flags & 0xF) << 8) + input_stream.read<u8>() + 3;
                count = 4;
            }
            output_ptr.append_from(-pos, count);
        }

        else
        {
            size_t count, pos;
            if (flags & 0x20)
            {
                pos = (flags & 0x1F) + 2;
                count = 2;
            }
            else
            {
                pos = (flags & 0x1F) + 1;
                count = 1;
            }
            output_ptr.append_from(-pos, count);
        }
    }

    return output;
}

bool ElgImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image ElgImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto header = read_header(input_file.stream);

    if (header.type == 2)
        while (input_file.stream.read<u8>() != 0)
            input_file.stream.skip(input_file.stream.read_le<u32>() - 4);

    if (header.depth == 8)
    {
        res::Palette palette(
            256,
            read_mono(256 * 4, 1, input_file.stream),
            res::PixelFormat::BGR888X);
        return res::Image(
            header.width,
            header.height,
            read_mono(header.width, header.height, input_file.stream),
            palette);
    }
    if (header.depth == 24)
    {
        return res::Image(
            header.width,
            header.height,
            read_rgb(header.width, header.height, input_file.stream),
            res::PixelFormat::BGR888);
    }
    else if (header.depth == 32)
    {
        res::Image output_image(
            header.width,
            header.height,
            read_rgb(header.width, header.height, input_file.stream),
            res::PixelFormat::BGR888);
        res::Image mask_image(
            header.width,
            header.height,
            read_mono(header.width, header.height, input_file.stream),
            res::PixelFormat::Gray8);
        output_image.apply_mask(mask_image);
        return output_image;
    }
    else
    {
        throw err::UnsupportedBitDepthError(header.depth);
    }
}

static auto _ = dec::register_decoder<ElgImageDecoder>("lucifen/elg");
