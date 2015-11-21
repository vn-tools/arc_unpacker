#include "fmt/french_bread/ex3_image_decoder.h"
#include "err.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::french_bread;

static const bstr magic = "LLIF"_b;

bool Ex3ImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<io::File> Ex3ImageDecoder::decode_impl(
    io::File &input_file) const
{
    input_file.stream.skip(magic.size());

    bstr output;
    bstr table0 = input_file.stream.read(0x40);
    bstr table1(256);
    bstr table2(256);

    u8 b = input_file.stream.read_u8();
    while (input_file.stream.tell() < input_file.stream.size())
    {
        for (auto j : util::range(256))
            table1[j] = j;

        size_t offset = 0;
        while (true)
        {
            if (b > 127)
            {
                offset += b - 127;
                b = 0;
            }
            if (offset == 256)
                break;
            for (u8 j : util::range(b + 1))
            {
                if (offset >= 256)
                    throw err::BadDataOffsetError();
                table1[offset] = input_file.stream.read_u8();
                if (offset != table1[offset])
                    table2[offset] = input_file.stream.read_u8();
                ++offset;
            }
            if (offset == 256)
                break;
            b = input_file.stream.read_u8();
        }

        int left = input_file.stream.read_u16_be();
        offset = 0;
        while (true)
        {
            if (offset)
            {
                --offset;
                if (offset >= 60)
                    throw err::BadDataOffsetError();
                b = table0[offset];
            }
            else
            {
                if (!left)
                    break;
                --left;
                b = input_file.stream.read_u8();
            }

            if (b == table1[b])
            {
                output += b;
            }
            else
            {
                if (offset >= 58)
                    throw err::BadDataOffsetError();
                table0[offset++] = table2[b];
                table0[offset++] = table1[b];
            }
        }
        if (input_file.stream.tell() < input_file.stream.size())
            b = input_file.stream.read_u8();
    }

    auto output_file = std::make_unique<io::File>();
    output_file->stream.write(output);
    output_file->name = input_file.name;
    output_file->change_extension(".bmp");
    return output_file;
}

static auto dummy = fmt::register_fmt<Ex3ImageDecoder>("french-bread/ex3");
