#include "fmt/liar_soft/cg_decompress.h"
#include "err.h"
#include "io/bit_reader.h"
#include "io/buffered_io.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::liar_soft;

void fmt::liar_soft::cg_decompress(
    bstr &output,
    const size_t output_offset,
    const size_t output_shift,
    io::IO &input_io,
    const size_t input_shift)
{
    auto output_ptr = output.get<u8>() + output_offset;
    const auto output_end = output.end<const u8>();
    if (output_shift < input_shift)
        throw std::logic_error("Invalid shift");

    const auto size_orig = input_io.read_u32_le();
    const auto size_comp = input_io.read_u32_le();
    const auto table_size = input_io.read_u16_le();
    if (!table_size)
        throw err::CorruptDataError("No table entries found");
    if (size_orig != ((output.size() / output_shift) * input_shift))
        throw err::BadDataSizeError();

    input_io.skip(2);
    const auto table = input_io.read(table_size * input_shift);

    const size_t unk1 = table_size < 0x1000 ? 6 : 0xE;
    const size_t unk2 = table_size < 0x1000 ? 3 : 4;

    io::BitReader bit_reader(input_io.read(size_comp));
    while (output_ptr < output_end)
    {
        try
        {
            size_t sequence_size = 1;
            size_t table_offset_size = bit_reader.get(unk2);

            if (!table_offset_size)
            {
                sequence_size = bit_reader.get(4) + 2;
                table_offset_size = bit_reader.get(unk2);
            }
            if (!table_offset_size)
                throw err::BadDataSizeError();

            size_t table_offset = 0;
            if (table_offset_size == 1)
            {
                table_offset = bit_reader.get(1);
            }
            else
            {
                table_offset_size--;
                if (table_offset_size >= unk1)
                {
                    while (!bit_reader.eof() && bit_reader.get(1))
                        ++table_offset_size;
                }
                table_offset = 1 << table_offset_size;
                table_offset |= bit_reader.get(table_offset_size);
            }

            if ((table_offset + 1) * input_shift > table.size())
                throw err::BadDataOffsetError();

            const auto input_chunk = table.substr(
                table_offset * input_shift, input_shift);
            for (auto i : util::range(sequence_size))
            {
                for (auto j : util::range(input_shift))
                {
                    if (output_ptr >= output_end)
                        break;
                    *output_ptr++ = input_chunk[j];
                }
                output_ptr += output_shift - input_shift;
            }
        }
        catch (err::EofError)
        {
            break;
        }
    }
}
