#include "fmt/twilight_frontier/pak1_archive_decoder.h"
#include "err.h"
#include "io/memory_stream.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::twilight_frontier;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

static void decrypt(bstr &buffer, u8 a, u8 b, u8 delta)
{
    for (auto i : util::range(buffer.size()))
    {
        buffer[i] ^= a;
        a += b;
        b += delta;
    }
}

static std::unique_ptr<io::MemoryStream> read_raw_table(
    io::Stream &arc_stream, size_t file_count)
{
    size_t table_size = file_count * 0x6C;
    if (table_size > arc_stream.size() - arc_stream.tell())
        throw err::RecognitionError();
    if (table_size > file_count * (0x64 + 4 + 4))
        throw err::RecognitionError();
    auto buffer = arc_stream.read(table_size);
    decrypt(buffer, 0x64, 0x64, 0x4D);
    return std::make_unique<io::MemoryStream>(buffer);
}

bool Pak1ArchiveDecoder::is_recognized_impl(io::File &input_file) const
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
    Pak1ArchiveDecoder::read_meta_impl(io::File &input_file) const
{
    u16 file_count = input_file.stream.read_u16_le();
    if (file_count == 0 && input_file.stream.size() != 6)
        throw err::RecognitionError();
    auto table_stream = read_raw_table(input_file.stream, file_count);
    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->path = table_stream->read_to_zero(0x64).str();
        entry->size = table_stream->read_u32_le();
        entry->offset = table_stream->read_u32_le();
        if (entry->offset + entry->size > input_file.stream.size())
            throw err::BadDataOffsetError();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> Pak1ArchiveDecoder::read_file_impl(
    io::File &input_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    auto output_file = std::make_unique<io::File>();
    output_file->path = entry->path;

    input_file.stream.seek(entry->offset);
    auto data = input_file.stream.read(entry->size);

    if (output_file->path.name() == "musicroom.dat")
    {
        decrypt(data, 0x5C, 0x5A, 0x3D);
        output_file->path.change_extension(".txt");
    }
    else if (output_file->path.has_extension("sce"))
    {
        decrypt(data, 0x63, 0x62, 0x42);
        output_file->path.change_extension(".txt");
    }
    else if (output_file->path.name() == "cardlist.dat")
    {
        decrypt(data, 0x60, 0x61, 0x41);
        output_file->path.change_extension(".txt");
    }

    output_file->stream.write(data);
    return output_file;
}

std::vector<std::string> Pak1ArchiveDecoder::get_linked_formats() const
{
    return {"twilight-frontier/pak1-sfx", "twilight-frontier/pak1-gfx"};
}

static auto dummy
    = fmt::register_fmt<Pak1ArchiveDecoder>("twilight-frontier/pak1");
