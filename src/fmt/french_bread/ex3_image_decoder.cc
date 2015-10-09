#include "fmt/french_bread/ex3_image_decoder.h"
#include "err.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::french_bread;

static const bstr magic = "LLIF"_b;

bool Ex3ImageDecoder::is_recognized_impl(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> Ex3ImageDecoder::decode_impl(File &file) const
{
    file.io.skip(magic.size());

    u8 table0[60];
    u8 table1[256];
    u8 table2[256];

    bstr output;
    file.io.read(table0, 0x40);

    u8 b = file.io.read_u8();
    while (file.io.tell() < file.io.size())
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
                table1[offset] = file.io.read_u8();
                if (offset != table1[offset])
                    table2[offset] = file.io.read_u8();
                ++offset;
            }
            if (offset == 256)
                break;
            b = file.io.read_u8();
        }

        int left = file.io.read_u16_be();
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
                b = file.io.read_u8();
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
        if (file.io.tell() < file.io.size())
            b = file.io.read_u8();
    }

    auto output_file = std::make_unique<File>();
    output_file->io.write(output);
    output_file->name = file.name;
    output_file->change_extension(".bmp");
    return output_file;
}

static auto dummy = fmt::Registry::add<Ex3ImageDecoder>("fbread/ex3");
