#include "fmt/leaf/pak_archive_decoder.h"
#include "err.h"
#include "util/encoding.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::leaf;

namespace
{
    struct TableEntry final
    {
        std::string name;
        u32 offset;
        bool compressed;
        u32 size;
    };

    using Table = std::vector<TableEntry>;
}

// Modified LZSS routine
// - starting position at 0 rather than 0xFEE
// - optionally, additional byte for repetition count
// - dictionary writing in two passes
static bstr custom_lzss_decompress(const bstr &input, size_t output_size)
{
    const size_t dict_capacity = 0x1000;
    u8 dict[dict_capacity] { 0 };
    size_t dict_size = 0;
    size_t dict_pos = 0;

    bstr output(output_size);
    auto output_ptr = output.get<u8>();
    auto output_end = output.end<const u8>();
    auto input_ptr = input.get<const u8>();
    auto input_end = input.end<const u8>();

    u16 control = 0;
    while (output_ptr < output_end)
    {
        control >>= 1;
        if (!(control & 0x100))
            control = *input_ptr++ | 0xFF00;

        if (control & 1)
        {
            dict[dict_pos++] = *output_ptr++ = *input_ptr++;
            dict_pos %= dict_capacity;
            if (dict_size < dict_capacity)
                dict_size++;
        }
        else
        {
            auto tmp = *reinterpret_cast<const u16*>(input_ptr);
            input_ptr += 2;

            auto look_behind_pos = tmp >> 4;
            auto repetitions = tmp & 0xF;
            if (repetitions == 0xF)
                repetitions += *input_ptr++;
            repetitions += 3;

            auto i = repetitions;
            while (i-- && output_ptr < output_end)
            {
                *output_ptr++ = dict[look_behind_pos++];
                look_behind_pos %= dict_size;
            }

            auto source = &output_ptr[-repetitions];
            while (source < output_ptr)
            {
                dict[dict_pos++] = *source++;
                dict_pos %= dict_capacity;
                if (dict_size < dict_capacity)
                    dict_size++;
            }
        }
    }

    return output;
}

static Table read_table(io::IO &arc_io)
{
    auto file_count = arc_io.read_u32_le();
    Table table;
    for (auto i : util::range(file_count))
    {
        TableEntry entry;
        entry.name = util::sjis_to_utf8(arc_io.read_to_zero(16)).str();
        entry.size = arc_io.read_u32_le();
        entry.compressed = arc_io.read_u32_le() > 0;
        entry.offset = arc_io.read_u32_le();
        if (entry.size)
            table.push_back(entry);
    }
    return table;
}

static std::unique_ptr<File> read_file(io::IO &arc_io, const TableEntry &entry)
{
    arc_io.seek(entry.offset);
    bstr data;
    if (entry.compressed)
    {
        auto size_comp = arc_io.read_u32_le();
        auto size_orig = arc_io.read_u32_le();
        data = arc_io.read(size_comp - 8);
        data = custom_lzss_decompress(data, size_orig);
    }
    else
    {
        data = arc_io.read(entry.size);
    }

    std::unique_ptr<File> file(new File);
    file->name = entry.name;
    file->io.write(data);
    return file;
}

bool PakArchiveDecoder::is_recognized_internal(File &arc_file) const
{
    auto table = read_table(arc_file.io);
    if (!table.size())
        return false;
    auto &last_entry = table[table.size() - 1];
    return last_entry.offset + last_entry.size == arc_file.io.size();
}

void PakArchiveDecoder::unpack_internal(File &arc_file, FileSaver &saver) const
{
    auto table = read_table(arc_file.io);
    for (auto &entry : table)
        saver.save(read_file(arc_file.io, entry));
}

static auto dummy = fmt::Registry::add<PakArchiveDecoder>("leaf/pak");
