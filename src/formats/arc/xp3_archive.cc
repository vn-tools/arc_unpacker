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
#include <cstring>
#include <memory>
#include "formats/arc/xp3_archive.h"
#include "buffered_io.h"
#include "logger.h"
#include "string_ex.h"

namespace
{
    typedef struct Xp3UnpackContext
    {
        IO &arc_io;
        IO &table_io;

        Xp3UnpackContext(IO &arc_io, IO &table_io)
            : arc_io(arc_io), table_io(table_io)
        {
        }
    } Xp3UnpackContext;

    const char *xp3_magic = "XP3\r\n\x20\x0a\x1a\x8b\x67\x01";
    const size_t xp3_magic_length = 11;

    const char *file_magic = "File";
    const size_t file_magic_length = 4;

    const char *adlr_magic = "adlr";
    const size_t adlr_magic_length = 4;

    const char *info_magic = "info";
    const size_t info_magic_length = 4;

    const char *segm_magic = "segm";
    const size_t segm_magic_length = 4;

    int xp3_detect_version(IO &arc_io)
    {
        int version = 1;
        size_t old_pos = arc_io.tell();
        arc_io.seek(19);
        if (arc_io.read_u32_le() == 1)
            version = 2;
        arc_io.seek(old_pos);
        return version;
    }

    bool xp3_check_magic(
        IO &arc_io,
        const char *expected_magic,
        size_t length)
    {
        char *magic = new char[length];
        assert(magic != nullptr);
        arc_io.read(magic, length);
        bool ok = memcmp(magic, expected_magic, length) == 0;
        delete []magic;
        return ok;
    }

    uint64_t xp3_get_table_offset(IO &arc_io, int version)
    {
        if (version == 1)
            return arc_io.read_u64_le();
        uint64_t additional_header_offset = arc_io.read_u64_le();
        uint32_t minor_version = arc_io.read_u32_le();
        if (minor_version != 1)
        {
            log_error("XP3: Unexpected XP3 version: %d", minor_version);
            return false;
        }

        arc_io.seek(additional_header_offset & 0xfffffffff);
        arc_io.skip(1); // flags?
        arc_io.skip(8); // table size
        return arc_io.read_u64_le();
    }

    std::unique_ptr<IO> xp3_read_raw_table(IO &arc_io)
    {
        bool use_zlib = arc_io.read_u8();
        const uint64_t size_compressed = arc_io.read_u64_le();
        __attribute__((unused)) const uint64_t size_original = use_zlib
            ? arc_io.read_u64_le()
            : size_compressed;

        std::string compressed = arc_io.read(size_compressed);
        if (use_zlib)
        {
            std::string uncompressed = zlib_inflate(compressed);
            return std::unique_ptr<IO>(new BufferedIO(uncompressed));
        }
        return std::unique_ptr<IO>(new BufferedIO(compressed));
    }

    bool xp3_read_info_chunk(IO &table_io, VirtualFile &target_file)
    {
        if (!xp3_check_magic(table_io, info_magic, info_magic_length))
        {
            log_error("XP3: Expected INFO chunk");
            return false;
        }
        uint64_t info_chunk_size = table_io.read_u64_le();

        __attribute__((unused)) uint32_t info_flags = table_io.read_u32_le();
        __attribute__((unused)) uint64_t file_size_original
            = table_io.read_u64_le();
        __attribute__((unused)) uint64_t file_size_compressed
            = table_io.read_u64_le();

        size_t name_length = table_io.read_u16_le();

        char *name_utf16 = new char [name_length * 2];
        assert(name_utf16 != nullptr);
        table_io.read(name_utf16, name_length * 2);

        char *name_utf8;
        if (!convert_encoding(
            name_utf16, name_length * 2,
            &name_utf8, nullptr,
            "UTF-16LE", "UTF-8"))
        {
            assert(0);
        }
        assert(name_utf8 != nullptr);
        target_file.name = std::string(name_utf8);
        delete []name_utf8;

        delete []name_utf16;
        assert(name_length * 2 + 22 == info_chunk_size);
        return true;
    }

    bool xp3_read_segm_chunk(
        IO &table_io,
        IO &arc_io,
        VirtualFile &target_file)
    {
        if (!xp3_check_magic(table_io, segm_magic, segm_magic_length))
        {
            table_io.skip(-segm_magic_length);
            return false;
        }
        uint64_t segm_chunk_size = table_io.read_u64_le();
        assert(28 == segm_chunk_size);

        uint32_t segm_flags = table_io.read_u32_le();
        uint64_t data_offset = table_io.read_u64_le();
        uint64_t data_size_original = table_io.read_u64_le();
        uint64_t data_size_compressed = table_io.read_u64_le();
        arc_io.seek(data_offset);

        bool use_zlib = segm_flags & 7;
        if (use_zlib)
        {
            std::string data_compressed = arc_io.read(data_size_compressed);
            std::string data_uncompressed = zlib_inflate(data_compressed);
            target_file.io.write(data_uncompressed);
        }
        else
        {
            target_file.io.write_from_io(arc_io, data_size_original);
        }

        return true;
    }

    uint32_t xp3_read_adlr_chunk(IO &table_io, uint32_t *encryption_key)
    {
        assert(encryption_key != nullptr);

        if (!xp3_check_magic(table_io, adlr_magic, adlr_magic_length))
        {
            log_error("XP3: Expected ADLR chunk");
            return false;
        }
        uint64_t adlr_chunk_size = table_io.read_u64_le();
        assert(4 == adlr_chunk_size);

        *encryption_key = table_io.read_u32_le();
        return true;
    }

    std::unique_ptr<VirtualFile> xp3_read_file(void *context)
    {
        assert(context != nullptr);
        Xp3UnpackContext *unpack_context = (Xp3UnpackContext*)context;

        IO &arc_io = unpack_context->arc_io;
        IO &table_io = unpack_context->table_io;
        std::unique_ptr<VirtualFile> target_file(new VirtualFile());

        if (!xp3_check_magic(table_io, file_magic, file_magic_length))
            throw std::runtime_error("Expected FILE chunk");

        uint64_t file_chunk_size = table_io.read_u64_le();
        size_t file_chunk_start_offset = table_io.tell();

        if (!xp3_read_info_chunk(table_io, *target_file))
        {
            return false;
        }

        while (true)
        {
            if (!xp3_read_segm_chunk(table_io, arc_io, *target_file))
                break;
        }

        uint32_t encryption_key;
        if (!xp3_read_adlr_chunk(table_io, &encryption_key))
        {
            return false;
        }

        assert(table_io.tell() - file_chunk_start_offset == file_chunk_size);
        return target_file;
    }
}

bool Xp3Archive::unpack_internal(IO &arc_io, OutputFiles &output_files)
{
    if (!xp3_check_magic(arc_io, xp3_magic, xp3_magic_length))
    {
        log_error("XP3: Not an XP3 archive");
        return false;
    }

    int version = xp3_detect_version(arc_io);
    log_info("XP3: Version: %d", version);

    uint64_t table_offset = xp3_get_table_offset(arc_io, version);
    arc_io.seek((uint32_t)table_offset);
    std::unique_ptr<IO> table_io = xp3_read_raw_table(arc_io);

    Xp3UnpackContext unpack_context(arc_io, *table_io);
    while (table_io->tell() < table_io->size())
        output_files.save(&xp3_read_file, &unpack_context);
    return true;
}
