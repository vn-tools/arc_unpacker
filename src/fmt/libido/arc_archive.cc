// ARC archive
//
// Company:   Libido
// Engine:    -
// Extension: .arc
//
// Known games:
// - Cherry Boy, Innocent Girl

#include <algorithm>
#include "fmt/libido/arc_archive.h"
#include "io/buffered_io.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::libido;

namespace
{
    struct TableEntry
    {
        std::string name;
        size_t size_compressed;
        size_t size_original;
        size_t offset;
    };

    using Table = std::vector<std::unique_ptr<TableEntry>>;
}

static bstr decompress(const bstr &input, size_t output_size)
{
    bstr output;
    output.resize(output_size);

    const size_t dict_size = 0x1000;
    size_t dict_pos = 0xFEE;
    u8 dict[dict_size];
    for (auto i : util::range(dict_size))
        dict[i] = 0;

    u8 *output_ptr = output.get<u8>();
    const u8 *output_end = output.end<u8>();
    const u8 *input_ptr = input.get<const u8>();
    const u8 *input_end = input.end<const u8>();

    u16 control = 0;
    while (output_ptr < output_end && input_ptr < input_end)
    {
        control >>= 1;
        if (!(control & 0x100))
        {
            control = *input_ptr++ | 0xFF00;
            if (input_ptr == input_end)
                break;
        }

        if (control & 1)
        {
            dict[dict_pos++] = *output_ptr++ = *input_ptr++;
            dict_pos %= dict_size;
            if (input_ptr == input_end)
                break;
        }
        else
        {
            u8 tmp1 = *input_ptr++;
            if (input_ptr == input_end)
                break;
            u8 tmp2 = *input_ptr++;
            if (input_ptr == input_end)
                break;

            u16 look_behind_pos = (((tmp2 & 0xF0) << 4) | tmp1) % dict_size;
            u16 repetitions = (tmp2 & 0xF) + 3;
            while (repetitions-- && output_ptr < output_end)
            {
                dict[dict_pos++] = *output_ptr++ = dict[look_behind_pos++];
                look_behind_pos %= dict_size;
                dict_pos %= dict_size;
            }
        }
    }
    return output;
}

static Table read_table(io::IO &arc_io)
{
    Table table;
    u32 file_count = arc_io.read_u32_le();
    for (auto i : util::range(file_count))
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);
        auto tmp = arc_io.read(20);
        for (auto i : util::range(tmp.size()))
            tmp[i] ^= 0xFF;
        entry->name = tmp.str(true);
        entry->size_original = arc_io.read_u32_le();
        entry->size_compressed = arc_io.read_u32_le();
        entry->offset = arc_io.read_u32_le();
        table.push_back(std::move(entry));
    }
    return table;
}

static std::unique_ptr<File> read_file(io::IO &arc_io, const TableEntry &entry)
{
    std::unique_ptr<File> file(new File);

    arc_io.seek(entry.offset);
    auto compressed_data = arc_io.read(entry.size_compressed);
    auto decompressed_data = decompress(compressed_data, entry.size_original);
    file->io.write(decompressed_data);
    file->name = entry.name;
    return file;
}

bool ArcArchive::is_recognized_internal(File &arc_file) const
{
    auto file_count = arc_file.io.read_u32_le();
    if (file_count)
    {
        arc_file.io.skip((file_count - 1) * 32 + 24);
        arc_file.io.seek(arc_file.io.read_u32_le() + arc_file.io.read_u32_le());
    }
    else
        arc_file.io.skip(1);
    return arc_file.io.eof();
}

void ArcArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    Table table = read_table(arc_file.io);
    for (auto &entry : table)
        file_saver.save(read_file(arc_file.io, *entry));
}
