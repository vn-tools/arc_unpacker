#include "fmt/minato_soft/pac_archive_decoder.h"
#include "fmt/minato_soft/fil_image_decoder.h"
#include "io/bit_reader.h"
#include "io/buffered_io.h"
#include "util/encoding.h"
#include "util/pack/zlib.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::minato_soft;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size_orig;
        size_t size_comp;
    };
}

static const bstr magic = "PAC\x00"_b;

static int init_huffman(io::BitReader &bit_reader, u16 nodes[2][512], int &pos)
{
    if (bit_reader.get(1))
    {
        auto old_pos = pos;
        pos++;
        if (old_pos < 511)
        {
            nodes[0][old_pos] = init_huffman(bit_reader, nodes, pos);
            nodes[1][old_pos] = init_huffman(bit_reader, nodes, pos);
            return old_pos;
        }
        return -1;
    }
    return bit_reader.get(8);
}

static bstr decompress_table(const bstr &input, size_t output_size)
{
    bstr output(output_size);
    auto output_ptr = output.get<u8>();
    auto output_end = output.end<const u8>();
    io::BitReader bit_reader(input);

    u16 nodes[2][512];
    auto pos = 256;
    auto initial_pos = init_huffman(bit_reader, nodes, pos);

    while (output_ptr < output_end)
    {
        auto pos = initial_pos;
        while (pos >= 256 && pos <= 511)
            pos = nodes[bit_reader.get(1)][pos];

        *output_ptr++ = pos;
    }
    return output;
}

struct PacArchiveDecoder::Priv final
{
    FilImageDecoder fil_image_decoder;
};

PacArchiveDecoder::PacArchiveDecoder() : p(new Priv)
{
    add_decoder(&p->fil_image_decoder);
}

PacArchiveDecoder::~PacArchiveDecoder()
{
}

bool PacArchiveDecoder::is_recognized_internal(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    PacArchiveDecoder::read_meta(File &arc_file) const
{
    arc_file.io.seek(magic.size());
    size_t file_count = arc_file.io.read_u32_le();
    arc_file.io.seek(arc_file.io.size() - 4);
    size_t compressed_size = arc_file.io.read_u32_le();
    size_t uncompressed_size = file_count * 76;

    arc_file.io.seek(arc_file.io.size() - 4 - compressed_size);
    bstr compressed = arc_file.io.read(compressed_size);
    for (auto i : util::range(compressed.size()))
        compressed.get<u8>()[i] ^= 0xFF;

    io::BufferedIO table_io(decompress_table(compressed, uncompressed_size));
    table_io.seek(0);

    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = util::sjis_to_utf8(table_io.read_to_zero(0x40)).str();
        entry->offset = table_io.read_u32_le();
        entry->size_orig = table_io.read_u32_le();
        entry->size_comp = table_io.read_u32_le();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> PacArchiveDecoder::read_file(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);
    auto data = arc_file.io.read(entry->size_comp);
    if (entry->size_orig != entry->size_comp)
        data = util::pack::zlib_inflate(data);
    return std::make_unique<File>(entry->name, data);
}

static auto dummy = fmt::Registry::add<PacArchiveDecoder>("minato/pac");
