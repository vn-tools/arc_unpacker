#include "fmt/leaf/pak_archive_decoder.h"
#include <map>
#include "err.h"
#include "fmt/leaf/grp_image_decoder.h"
#include "util/encoding.h"
#include "util/file_from_grid.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::leaf;

namespace
{
    struct TableEntry final
    {
        std::string name;
        u32 offset;
        u32 size;
        bool compressed;
        bool already_unpacked;
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
        TableEntry entry { };
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

struct PakArchiveDecoder::Priv final
{
    GrpImageDecoder grp_image_decoder;
};

PakArchiveDecoder::PakArchiveDecoder() : p(new Priv)
{
}

PakArchiveDecoder::~PakArchiveDecoder()
{
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

    if (nested_decoding_enabled)
    {
        // decode GRP
        std::map<std::string, TableEntry*> palette_entries, sprite_entries;
        for (auto i : util::range(table.size()))
        {
            auto fn = table[i].name.substr(0, table[i].name.find_first_of('.'));
            if (table[i].name.find("c16") != std::string::npos)
                palette_entries[fn] = &table[i];
            else if (table[i].name.find("grp") != std::string::npos)
                sprite_entries[fn] = &table[i];
        }

        for (auto it : sprite_entries)
        {
            try
            {
                auto &sprite_entry = it.second;
                auto &palette_entry = palette_entries.at(it.first);
                auto sprite_file = read_file(arc_file.io, *sprite_entry);
                auto palette_file = read_file(arc_file.io, *palette_entry);
                auto sprite
                    = p->grp_image_decoder.decode(*sprite_file, *palette_file);
                sprite_entry->already_unpacked = true;
                palette_entry->already_unpacked = true;
                saver.save(util::file_from_grid(sprite, sprite_entry->name));
            }
            catch (...)
            {
                continue;
            }
        }
    }

    for (auto &entry : table)
        if (!entry.already_unpacked)
            saver.save(read_file(arc_file.io, entry));
}

static auto dummy = fmt::Registry::add<PakArchiveDecoder>("leaf/pak");
