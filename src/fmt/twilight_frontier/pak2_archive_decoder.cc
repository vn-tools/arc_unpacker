#include "fmt/twilight_frontier/pak2_archive_decoder.h"
#include "err.h"
#include "fmt/twilight_frontier/pak2_image_decoder.h"
#include "io/filesystem.h"
#include "io/memory_stream.h"
#include "util/encoding.h"
#include "util/file_from_grid.h"
#include "util/mt.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::twilight_frontier;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
        bool already_unpacked;
    };
}

static void decrypt(bstr &buffer, u32 mt_seed, u8 a, u8 b, u8 delta)
{
    util::mt::init_genrand(mt_seed);
    for (auto i : util::range(buffer.size()))
    {
        buffer[i] ^= util::mt::genrand_int32();
        buffer[i] ^= a;
        a += b;
        b += delta;
    }
}

bool Pak2ArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    try
    {
        read_meta(input_file);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

std::unique_ptr<fmt::ArchiveMeta>
    Pak2ArchiveDecoder::read_meta_impl(io::File &input_file) const
{
    input_file.stream.seek(0);
    u16 file_count = input_file.stream.read_u16_le();
    if (file_count == 0 && input_file.stream.size() != 6)
        throw err::RecognitionError();

    size_t table_size = input_file.stream.read_u32_le();
    if (table_size > input_file.stream.size() - input_file.stream.tell())
        throw err::RecognitionError();
    if (table_size > file_count * (4 + 4 + 256 + 1))
        throw err::RecognitionError();
    auto table_data = input_file.stream.read(table_size);
    decrypt(table_data, table_size + 6, 0xC5, 0x83, 0x53);
    io::MemoryStream table_stream(table_data);

    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->already_unpacked = false;
        entry->offset = table_stream.read_u32_le();
        entry->size = table_stream.read_u32_le();
        auto name_size = table_stream.read_u8();
        entry->name = util::sjis_to_utf8(table_stream.read(name_size)).str();
        if (entry->offset + entry->size > input_file.stream.size())
            throw err::BadDataOffsetError();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> Pak2ArchiveDecoder::read_file_impl(
    io::File &input_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    if (entry->already_unpacked)
        return nullptr;
    input_file.stream.seek(entry->offset);
    auto data = input_file.stream.read(entry->size);
    u8 key = (entry->offset >> 1) | 0x23;
    for (auto i : util::range(entry->size))
        data[i] ^= key;
    return std::make_unique<io::File>(entry->name, data);
}

void Pak2ArchiveDecoder::preprocess(
    io::File &input_file, ArchiveMeta &m, const FileSaver &saver) const
{
    Pak2ImageDecoder image_decoder;

    image_decoder.clear_palettes();
    auto dir = io::path(input_file.name).parent();
    for (const auto &path : io::directory_range(dir))
    {
        if (!io::is_regular_file(path))
            continue;
        if (path.str().find(".dat") == std::string::npos)
            continue;

        io::File other_input_file(path, io::FileMode::Read);
        if (!is_recognized(other_input_file))
            continue;

        auto meta = read_meta(other_input_file);
        for (auto &entry : meta->entries)
        {
            if (entry->name.find(".pal") == std::string::npos)
                continue;

            auto pal_file = read_file(other_input_file, *meta, *entry);
            pal_file->stream.seek(0);
            image_decoder.add_palette(
                entry->name, pal_file->stream.read_to_eof());
        }
    }

    for (auto &e : m.entries)
    {
        auto entry = static_cast<ArchiveEntryImpl*>(e.get());
        if (entry->name.find(".cv2") == std::string::npos)
            continue;
        auto full_file = read_file(input_file, m, *entry);
        try
        {
            auto pixels = image_decoder.decode(*full_file);
            saver.save(util::file_from_grid(pixels, entry->name));
        }
        catch (...)
        {
        }
    }
}

std::vector<std::string> Pak2ArchiveDecoder::get_linked_formats() const
{
    return {"twilight-frontier/pak2-sfx", "twilight-frontier/pak2-gfx"};
}

static auto dummy
    = fmt::register_fmt<Pak2ArchiveDecoder>("twilight-frontier/pak2");
