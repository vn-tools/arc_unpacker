#include "fmt/twilight_frontier/pak2_archive_decoder.h"
#include "algo/binary.h"
#include "algo/crypt/mt.h"
#include "algo/locale.h"
#include "algo/range.h"
#include "err.h"
#include "fmt/twilight_frontier/pak2_image_decoder.h"
#include "io/file_system.h"
#include "io/memory_stream.h"
#include "util/file_from_image.h"

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
    auto mt = algo::crypt::MersenneTwister::Improved(mt_seed);
    for (const auto i : algo::range(buffer.size()))
    {
        buffer[i] ^= mt->next_u32();
        buffer[i] ^= a;
        a += b;
        b += delta;
    }
}

bool Pak2ArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    Logger dummy_logger;
    dummy_logger.mute();
    try
    {
        read_meta(dummy_logger, input_file);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

std::unique_ptr<fmt::ArchiveMeta> Pak2ArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(0);
    const auto file_count = input_file.stream.read_u16_le();
    if (!file_count && input_file.stream.size() != 6)
        throw err::RecognitionError();

    const auto table_size = input_file.stream.read_u32_le();
    if (table_size > input_file.stream.size() - input_file.stream.tell())
        throw err::RecognitionError();
    if (table_size > file_count * (4 + 4 + 256 + 1))
        throw err::RecognitionError();
    auto table_data = input_file.stream.read(table_size);
    decrypt(table_data, table_size + 6, 0xC5, 0x83, 0x53);
    io::MemoryStream table_stream(table_data);

    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->already_unpacked = false;
        entry->offset = table_stream.read_u32_le();
        entry->size = table_stream.read_u32_le();
        const auto name_size = table_stream.read_u8();
        entry->path = algo::sjis_to_utf8(table_stream.read(name_size)).str();
        if (entry->offset + entry->size > input_file.stream.size())
            throw err::BadDataOffsetError();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> Pak2ArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const fmt::ArchiveMeta &m,
    const fmt::ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    if (entry->already_unpacked)
        return nullptr;
    const auto data = algo::xor(
        input_file.stream.seek(entry->offset).read(entry->size),
        (entry->offset >> 1) | 0x23);
    return std::make_unique<io::File>(entry->path, data);
}

void Pak2ArchiveDecoder::preprocess(
    const Logger &logger,
    io::File &input_file,
    fmt::ArchiveMeta &m,
    const FileSaver &saver) const
{
    Pak2ImageDecoder image_decoder;

    image_decoder.clear_palettes();
    const auto dir = input_file.path.parent();
    for (const auto &path : io::directory_range(dir))
    {
        if (!io::is_regular_file(path))
            continue;
        if (!path.has_extension("dat"))
            continue;

        io::File other_input_file(path, io::FileMode::Read);
        if (!is_recognized(other_input_file))
            continue;

        auto meta = read_meta(logger, other_input_file);
        for (auto &entry : meta->entries)
        {
            if (!entry->path.has_extension("pal"))
                continue;

            auto pal_file = read_file(logger, other_input_file, *meta, *entry);
            image_decoder.add_palette(
                entry->path, pal_file->stream.seek(0).read_to_eof());
        }
    }

    for (auto &e : m.entries)
    {
        auto entry = static_cast<ArchiveEntryImpl*>(e.get());
        if (!entry->path.has_extension("cv2"))
            continue;
        auto full_file = read_file(logger, input_file, m, *entry);
        try
        {
            auto image = image_decoder.decode(logger, *full_file);
            saver.save(util::file_from_image(image, entry->path));
            entry->already_unpacked = true;
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
