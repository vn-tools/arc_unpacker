// XP3 archive
//
// Company:   -
// Engine:    Kirikiri
// Extension: .xp3
//
// Known games:
// - [Akabei Soft2] [051125] Sharin no Kuni, Himawari no Shoujo
// - [Akabei Soft2] [080529] G-senjou no Maou
// - [Akatsuki Works] [091022] Comyu Kuroi Ryuu to Yasashii Oukoku
// - [Parasol] [120525] Koiimo - SWEET DAYS
// - [Parasol] [140829] Hare Nochi Kitto Nanohana Biyori
// - [Pita Fetish] [110428] Gokkun! Onii-chan Milk ~Punipuni Oppai na Imouto to~
// - [Type-Moon] [040326] Fate Stay Night
// - [Type-Moon] [051229] Fate Hollow Ataraxia
// - [Type-Moon] [120412] Mahou Tsukai no Yoru
// - [Yurin Yurin] [121130] Sonohana 12 - Atelier no Koibito-tachi

#include "fmt/kirikiri/tlg_converter.h"
#include "fmt/kirikiri/xp3_archive.h"
#include "fmt/kirikiri/xp3_filter_registry.h"
#include "io/buffered_io.h"
#include "util/encoding.h"
#include "util/pack/zlib.h"
#include "util/require.h"

using namespace au;
using namespace au::fmt::kirikiri;

namespace
{
    struct InfoChunk
    {
        u32 flags;
        u64 file_size_original;
        u64 file_size_compressed;
        std::string name;
    };
}

static const bstr xp3_magic = "XP3\r\n\x20\x0A\x1A\x8B\x67\x01"_b;
static const bstr file_magic = "File"_b;
static const bstr adlr_magic = "adlr"_b;
static const bstr info_magic = "info"_b;
static const bstr segm_magic = "segm"_b;

static int detect_version(io::IO &arc_io)
{
    int version = 1;
    size_t old_pos = arc_io.tell();
    arc_io.seek(19);
    if (arc_io.read_u32_le() == 1)
        version = 2;
    arc_io.seek(old_pos);
    return version;
}

static u64 get_table_offset(io::IO &arc_io, int version)
{
    if (version == 1)
        return arc_io.read_u64_le();

    u64 additional_header_offset = arc_io.read_u64_le();
    u32 minor_version = arc_io.read_u32_le();
    if (minor_version != 1)
        throw std::runtime_error("Unexpected XP3 version");

    arc_io.seek(additional_header_offset);
    arc_io.skip(1); // flags?
    arc_io.skip(8); // table size
    return arc_io.read_u64_le();
}

static std::unique_ptr<io::IO> read_raw_table(io::IO &arc_io)
{
    bool use_zlib = arc_io.read_u8() != 0;
    const u64 size_compressed = arc_io.read_u64_le();
    const u64 size_original = use_zlib
        ? arc_io.read_u64_le()
        : size_compressed;

    bstr data = arc_io.read(size_compressed);
    if (use_zlib)
        data = util::pack::zlib_inflate(data);
    return std::unique_ptr<io::IO>(new io::BufferedIO(data));
}

static InfoChunk read_info_chunk(io::IO &table_io)
{
    if (table_io.read(info_magic.size()) != info_magic)
        throw std::runtime_error("Expected INFO chunk");
    u64 info_chunk_size = table_io.read_u64_le();

    InfoChunk info_chunk;
    info_chunk.flags = table_io.read_u32_le();
    info_chunk.file_size_original = table_io.read_u64_le();
    info_chunk.file_size_compressed = table_io.read_u64_le();

    size_t file_name_size = table_io.read_u16_le();
    auto name = table_io.read(file_name_size * 2);
    info_chunk.name = util::convert_encoding(name, "utf-16le", "utf-8").str();
    return info_chunk;
}

static bstr read_data_from_segm_chunk(io::IO &table_io, io::IO &arc_io)
{
    if (table_io.read(segm_magic.size()) != segm_magic)
        throw std::runtime_error("Expected SEGM chunk");

    u64 segm_chunk_size = table_io.read_u64_le();
    if (segm_chunk_size % 28 != 0)
        throw std::runtime_error("Unexpected SEGM chunk size");
    bstr full_data;
    size_t initial_pos = table_io.tell();
    while (segm_chunk_size > table_io.tell() - initial_pos)
    {
        u32 segm_flags = table_io.read_u32_le();
        u64 data_offset = table_io.read_u64_le();
        u64 data_size_original = table_io.read_u64_le();
        u64 data_size_compressed = table_io.read_u64_le();
        arc_io.seek(data_offset);

        bool use_zlib = (segm_flags & 7) > 0;
        if (use_zlib)
        {
            auto data_compressed = arc_io.read(data_size_compressed);
            auto data_uncompressed = util::pack::zlib_inflate(data_compressed);
            full_data += data_uncompressed;
        }
        else
        {
            full_data += arc_io.read(data_size_original);
        }
    }

    return full_data;
}

static u32 read_key_from_adlr_chunk(io::IO &table_io)
{
    if (table_io.read(adlr_magic.size()) != adlr_magic)
        throw std::runtime_error("Expected ADLR chunk");

    u64 adlr_chunk_size = table_io.read_u64_le();
    if (adlr_chunk_size != 4)
        throw std::runtime_error("Unexpected ADLR chunk size");

    return table_io.read_u32_le();
}

static std::unique_ptr<File> read_file(
    io::IO &arc_io, io::IO &table_io, const Xp3FilterFunc &filter_func)
{
    std::unique_ptr<File> target_file(new File());

    if (table_io.read(file_magic.size()) != file_magic)
        throw std::runtime_error("Expected FILE chunk");

    u64 file_chunk_size = table_io.read_u64_le();
    size_t file_chunk_start_offset = table_io.tell();

    auto info_chunk = read_info_chunk(table_io);
    auto data = read_data_from_segm_chunk(table_io, arc_io);
    auto key = read_key_from_adlr_chunk(table_io);

    util::require(table_io.tell() - file_chunk_start_offset == file_chunk_size);

    if (filter_func)
        filter_func(data, key);

    target_file->name = info_chunk.name;
    target_file->io.write(data);
    return target_file;
}

struct Xp3Archive::Priv
{
    Xp3FilterRegistry filter_registry;
    TlgConverter tlg_converter;
};

Xp3Archive::Xp3Archive() : p(new Priv)
{
    add_transformer(&p->tlg_converter);
}

Xp3Archive::~Xp3Archive()
{
}

void Xp3Archive::register_cli_options(ArgParser &arg_parser) const
{
    p->filter_registry.register_cli_options(arg_parser);
    Archive::register_cli_options(arg_parser);
}

void Xp3Archive::parse_cli_options(const ArgParser &arg_parser)
{
    p->filter_registry.parse_cli_options(arg_parser);
    Archive::parse_cli_options(arg_parser);
}

bool Xp3Archive::is_recognized_internal(File &arc_file) const
{
    return arc_file.io.read(xp3_magic.size()) == xp3_magic;
}

void Xp3Archive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    arc_file.io.skip(xp3_magic.size());

    int version = detect_version(arc_file.io);
    u64 table_offset = get_table_offset(arc_file.io, version);
    arc_file.io.seek(table_offset);
    auto table_io = read_raw_table(arc_file.io);

    Xp3Filter filter(arc_file.name);
    p->filter_registry.set_decoder(filter);

    while (table_io->tell() < table_io->size())
        file_saver.save(read_file(arc_file.io, *table_io, filter.decoder));
}

static auto dummy = fmt::Registry::add<Xp3Archive>("krkr/xp3");
