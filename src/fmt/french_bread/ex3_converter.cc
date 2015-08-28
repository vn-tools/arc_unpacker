// EX3 image
//
// Company:   French Bread
// Engine:    -
// Extension: .ex3
// Archives:  .p
//
// Known games:
// - Melty Blood

#include <stdexcept>
#include "fmt/french_bread/ex3_converter.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::french_bread;

static const bstr magic = "LLIF"_b;

bool Ex3Converter::is_recognized_internal(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> Ex3Converter::decode_internal(File &file) const
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
                    throw std::runtime_error("Bad offset");
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
                    throw std::runtime_error("Bad offset");
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
                    throw std::runtime_error("Bad offset");
                table0[offset++] = table2[b];
                table0[offset++] = table1[b];
            }
        }
        if (file.io.tell() < file.io.size())
            b = file.io.read_u8();
    }

    std::unique_ptr<File> output_file(new File);
    output_file->io.write(output);
    output_file->name = file.name;
    output_file->change_extension(".bmp");
    return output_file;
}

static auto dummy = fmt::Registry::add<Ex3Converter>("fbread/ex3");
