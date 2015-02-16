// XP3 archive
//
// Company:   -
// Engine:    Kirikiri
// Extension: .xp3
//
// Known games:
// - Fate/Stay Night
// - Fate/Hollow Ataraxia
// - Sono Hanabira ni Kuchizuke o 12
// - Sharin no Kuni, Himawari no Shoujo
// - Comyu Kuroi Ryuu to Yasashii Oukoku

#include <cassert>
#include "buffered_io.h"
#include "formats/arc/xp3_archive.h"
#include "formats/arc/xp3_archive/xp3_filter.h"
#include "formats/arc/xp3_archive/xp3_filter_comyu.h"
#include "formats/arc/xp3_archive/xp3_filter_fha.h"
#include "formats/arc/xp3_archive/xp3_filter_fsn.h"
#include "formats/arc/xp3_archive/xp3_filter_noop.h"
#include "formats/gfx/tlg_converter.h"
#include "string_ex.h"

namespace
{
    const std::string xp3_magic("XP3\r\n\x20\x0a\x1a\x8b\x67\x01", 11);
    const std::string file_magic("File", 4);
    const std::string adlr_magic("adlr", 4);
    const std::string info_magic("info", 4);
    const std::string segm_magic("segm", 4);

    int detect_version(IO &arc_io)
    {
        int version = 1;
        size_t old_pos = arc_io.tell();
        arc_io.seek(19);
        if (arc_io.read_u32_le() == 1)
            version = 2;
        arc_io.seek(old_pos);
        return version;
    }

    bool check_magic(IO &arc_io, std::string expected_magic)
    {
        return arc_io.read(expected_magic.size()) == expected_magic;
    }

    uint64_t get_table_offset(IO &arc_io, int version)
    {
        if (version == 1)
            return arc_io.read_u64_le();

        uint64_t additional_header_offset = arc_io.read_u64_le();
        uint32_t minor_version = arc_io.read_u32_le();
        if (minor_version != 1)
            throw std::runtime_error("Unexpected XP3 version");

        arc_io.seek(static_cast<size_t>(additional_header_offset));
        arc_io.skip(1); // flags?
        arc_io.skip(8); // table size
        return arc_io.read_u64_le();
    }

    std::unique_ptr<IO> read_raw_table(IO &arc_io)
    {
        bool use_zlib = arc_io.read_u8() != 0;
        const uint64_t size_compressed = arc_io.read_u64_le();
        const uint64_t size_original = use_zlib
            ? arc_io.read_u64_le()
            : size_compressed;

        std::string compressed
            = arc_io.read(static_cast<size_t>(size_compressed));
        if (use_zlib)
        {
            std::string uncompressed = zlib_inflate(compressed);
            return std::unique_ptr<IO>(new BufferedIO(uncompressed));
        }
        return std::unique_ptr<IO>(new BufferedIO(compressed));
    }

    void read_info_chunk(IO &table_io, File &target_file)
    {
        if (!check_magic(table_io, info_magic))
            throw std::runtime_error("Expected INFO chunk");
        uint64_t info_chunk_size = table_io.read_u64_le();

        uint32_t info_flags = table_io.read_u32_le();
        uint64_t file_size_original = table_io.read_u64_le();
        uint64_t file_size_compressed = table_io.read_u64_le();

        size_t name_length = table_io.read_u16_le();
        std::string name_utf16 = table_io.read(name_length * 2);
        target_file.name = convert_encoding(name_utf16, "UTF-16LE", "UTF-8");
        assert(info_chunk_size == name_length * 2 + 22);
    }

    bool read_segm_chunk(
        IO &table_io,
        IO &arc_io,
        File &target_file)
    {
        if (!check_magic(table_io, segm_magic))
        {
            table_io.skip(-(signed)segm_magic.size());
            return false;
        }
        uint64_t segm_chunk_size = table_io.read_u64_le();
        assert(segm_chunk_size == 28);

        uint32_t segm_flags = table_io.read_u32_le();
        uint64_t data_offset = table_io.read_u64_le();
        uint64_t data_size_original = table_io.read_u64_le();
        uint64_t data_size_compressed = table_io.read_u64_le();
        arc_io.seek(static_cast<size_t>(data_offset));

        bool use_zlib = (segm_flags & 7) > 0;
        if (use_zlib)
        {
            std::string data_compressed = arc_io.read(
                static_cast<int>(data_size_compressed));
            std::string data_uncompressed = zlib_inflate(data_compressed);
            target_file.io.write(data_uncompressed);
        }
        else
        {
            target_file.io.write_from_io(
                arc_io, static_cast<size_t>(data_size_original));
        }

        return true;
    }

    uint32_t read_adlr_chunk(IO &table_io, uint32_t *encryption_key)
    {
        assert(encryption_key != nullptr);

        if (!check_magic(table_io, adlr_magic))
            throw std::runtime_error("Expected ADLR chunk");

        uint64_t adlr_chunk_size = table_io.read_u64_le();
        assert(adlr_chunk_size == 4);

        *encryption_key = table_io.read_u32_le();
        return true;
    }

    std::unique_ptr<File> read_file(
        IO &arc_io,
        IO &table_io,
        TlgConverter &tlg_converter,
        Xp3Filter *filter)
    {
        std::unique_ptr<File> target_file(new File());

        if (!check_magic(table_io, file_magic))
            throw std::runtime_error("Expected FILE chunk");

        uint64_t file_chunk_size = table_io.read_u64_le();
        size_t file_chunk_start_offset = table_io.tell();

        read_info_chunk(table_io, *target_file);

        while (read_segm_chunk(table_io, arc_io, *target_file))
        {
        }

        uint32_t encryption_key;
        read_adlr_chunk(table_io, &encryption_key);

        assert(table_io.tell() - file_chunk_start_offset == file_chunk_size);

        if (filter != nullptr)
            filter->decode(*target_file, encryption_key);

        tlg_converter.try_decode(*target_file);

        return target_file;
    }
}

struct Xp3Archive::Internals
{
    TlgConverter tlg_converter;
    std::unique_ptr<Xp3Filter> filter;

    Internals() : filter(nullptr)
    {
    }

    ~Internals()
    {
    }
};

Xp3Archive::Xp3Archive() : internals(new Internals)
{
}

Xp3Archive::~Xp3Archive()
{
}

void Xp3Archive::add_cli_help(ArgParser &arg_parser) const
{
    arg_parser.add_help(
        "--plugin=PLUGIN",
        "Selects XP3 decryption routine.\n"
            "Possible values:\n"
            "- comyu\n"
            "- fha\n"
            "- fsn\n"
            "- noop (for unecrypted games)");
    internals->tlg_converter.add_cli_help(arg_parser);
}

void Xp3Archive::parse_cli_options(ArgParser &arg_parser)
{
    const std::string plugin = arg_parser.get_switch("plugin").c_str();
    if (plugin == "comyu")
        internals->filter.reset(new Xp3FilterComyu);
    else if (plugin == "fha")
        internals->filter.reset(new Xp3FilterFha);
    else if (plugin == "fsn")
        internals->filter.reset(new Xp3FilterFsn);
    else if (plugin == "noop")
        internals->filter.reset(new Xp3FilterNoop);
    else
        throw std::runtime_error("Unrecognized plugin: " + plugin);

    internals->tlg_converter.parse_cli_options(arg_parser);
}

void Xp3Archive::unpack_internal(File &file, FileSaver &file_saver) const
{
    if (!check_magic(file.io, xp3_magic))
        throw std::runtime_error("Not an XP3 archive");

    int version = detect_version(file.io);
    uint64_t table_offset = get_table_offset(file.io, version);
    file.io.seek((uint32_t)table_offset);
    std::unique_ptr<IO> table_io = read_raw_table(file.io);

    while (table_io->tell() < table_io->size())
    {
        file_saver.save(read_file(
            file.io,
            *table_io,
            internals->tlg_converter,
            internals->filter.get()));
    }
}
