#include "fmt/kid/decompressor.h"
#include "io/buffered_io.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;

std::unique_ptr<io::IO> au::fmt::kid::decompress(
    io::IO &input_io, size_t size_original)
{
    std::unique_ptr<io::BufferedIO> output_io(new io::BufferedIO);
    output_io->reserve(size_original);
    while (output_io->tell() < size_original)
    {
        if (input_io.eof())
        {
            throw std::runtime_error(util::format(
                "EOF reached at %d/%d", output_io->tell(), size_original));
        }
        u8 byte = input_io.read_u8();
        if (byte & 0x80)
        {
            if (byte & 0x40)
            {
                int repetitions = (byte & 0x1F) + 2;
                if (byte & 0x20)
                    repetitions += input_io.read_u8() << 5;
                u8 data_byte = input_io.read_u8();
                for (auto i : util::range(repetitions))
                    output_io->write_u8(data_byte);
            }
            else
            {
                int length = ((byte >> 2) & 0xF) + 2;
                int look_behind = ((byte & 3) << 8) + input_io.read_u8() + 1;
                size_t start_pos = output_io->tell() - look_behind;
                if (start_pos > output_io->tell())
                    start_pos = output_io->tell();
                for (auto i : util::range(length))
                    output_io->write_u8(output_io->buffer()[start_pos + i]);
            }
        }
        else
        {
            if (byte & 0x40)
            {
                int repetitions = input_io.read_u8() + 1;
                int length = (byte & 0x3F) + 2;
                auto chunk = input_io.read(length);
                for (auto i : util::range(repetitions))
                    output_io->write(chunk);
            }
            else
            {
                int length = (byte & 0x1F) + 1;
                if (byte & 0x20)
                    length += input_io.read_u8() << 5;
                output_io->write(input_io.read(length));
            }
        }
    }
    output_io->seek(0);
    return std::unique_ptr<io::IO>(std::move(output_io));
}
