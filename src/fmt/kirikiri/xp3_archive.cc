// XP3 archive
//
// Company:   -
// Engine:    Kirikiri
// Extension: .xp3
//
// Known games:
// - Comyu Kuroi Ryuu to Yasashii Oukoku
// - Fate/Hollow Ataraxia
// - Fate/Stay Night
// - G-Senjou no Maou
// - Hare Nochi Kitto Nanohana Biyori
// - Koiimo SWEET☆DAYS
// - Mahou Tsukai no Yoru
// - Sharin no Kuni, Himawari no Shoujo
// - Sono Hanabira ni Kuchizuke o 12 - Atelier no Koibito-tachi

#include "fmt/kirikiri/tlg_converter.h"
#include "fmt/kirikiri/xp3_archive.h"
#include "fmt/kirikiri/xp3_filter_factory.h"
#include "io/buffered_io.h"
#include "util/encoding.h"
#include "util/pack/zlib.h"

using namespace au;
using namespace au::fmt::kirikiri;

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

static void read_info_chunk(io::IO &table_io, File &target_file)
{
    if (table_io.read(info_magic.size()) != info_magic)
        throw std::runtime_error("Expected INFO chunk");
    u64 info_chunk_size = table_io.read_u64_le();

    u32 info_flags = table_io.read_u32_le();
    u64 file_size_original = table_io.read_u64_le();
    u64 file_size_compressed = table_io.read_u64_le();

    size_t file_name_size = table_io.read_u16_le();
    bstr file_name_utf16 = table_io.read(file_name_size * 2);
    target_file.name
        = util::convert_encoding(file_name_utf16, "utf-16le", "utf-8").str();
    if (info_chunk_size != file_name_size * 2 + 22)
        throw std::runtime_error("Unexpected INFO chunk size");
}

static bool read_segm_chunk(io::IO &table_io, io::IO &arc_io, File &target_file)
{
    if (table_io.read(segm_magic.size()) != segm_magic)
        throw std::runtime_error("Expected SEGM chunk");

    u64 segm_chunk_size = table_io.read_u64_le();
    if (segm_chunk_size % 28 != 0)
        throw std::runtime_error("Unexpected SEGM chunk size");
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
            target_file.io.write(data_uncompressed);
        }
        else
        {
            target_file.io.write_from_io(arc_io, data_size_original);
        }
    }

    return true;
}

static u32 read_adlr_chunk(io::IO &table_io, u32 *encryption_key)
{
    if (table_io.read(adlr_magic.size()) != adlr_magic)
        throw std::runtime_error("Expected ADLR chunk");

    u64 adlr_chunk_size = table_io.read_u64_le();
    if (adlr_chunk_size != 4)
        throw std::runtime_error("Unexpected ADLR chunk size");

    *encryption_key = table_io.read_u32_le();
    return true;
}

static std::unique_ptr<File> read_file(
    io::IO &arc_io, io::IO &table_io, Xp3Filter *filter)
{
    std::unique_ptr<File> target_file(new File());

    if (table_io.read(file_magic.size()) != file_magic)
        throw std::runtime_error("Expected FILE chunk");

    u32 encryption_key;
    u64 file_chunk_size = table_io.read_u64_le();
    size_t file_chunk_start_offset = table_io.tell();

    read_info_chunk(table_io, *target_file);
    read_segm_chunk(table_io, arc_io, *target_file);
    read_adlr_chunk(table_io, &encryption_key);

    if (table_io.tell() - file_chunk_start_offset != file_chunk_size)
        throw std::runtime_error("Unexpected FILE chunk size");

    if (filter)
        filter->decode(*target_file, encryption_key);

    return target_file;
}

struct Xp3Archive::Priv
{
    Xp3FilterFactory filter_factory;
    TlgConverter tlg_converter;
    std::unique_ptr<Xp3Filter> filter;

    Priv() : filter(nullptr)
    {
    }
};

Xp3Archive::Xp3Archive() : p(new Priv)
{
    add_transformer(&p->tlg_converter);
}

Xp3Archive::~Xp3Archive()
{
}

void Xp3Archive::add_cli_help(ArgParser &arg_parser) const
{
    p->filter_factory.add_cli_help(arg_parser);
    Archive::add_cli_help(arg_parser);
}

void Xp3Archive::parse_cli_options(const ArgParser &arg_parser)
{
    p->filter = p->filter_factory.get_filter_from_cli_options(arg_parser);
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

    if (p->filter)
        p->filter->set_arc_path(arc_file.name);

    while (table_io->tell() < table_io->size())
        file_saver.save(read_file(arc_file.io, *table_io, p->filter.get()));
}

static auto dummy = fmt::Registry::add<Xp3Archive>("krkr/xp3");
