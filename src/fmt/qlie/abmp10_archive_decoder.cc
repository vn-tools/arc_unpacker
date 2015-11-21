#include "fmt/qlie/abmp10_archive_decoder.h"
#include "err.h"
#include "util/encoding.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::qlie;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

static const bstr magic10 = "abmp10\0\0\0\0\0\0\0\0\0\0"_b;
static const bstr magic11 = "abmp11\0\0\0\0\0\0\0\0\0\0"_b;
static const bstr magic12 = "abmp12\0\0\0\0\0\0\0\0\0\0"_b;
static const bstr magic_imgdat10 = "abimgdat10\0\0\0\0\0\0"_b;
static const bstr magic_imgdat11 = "abimgdat11\0\0\0\0\0\0"_b;
static const bstr magic_imgdat13 = "abimgdat13\0\0\0\0\0\0"_b;
static const bstr magic_imgdat14 = "abimgdat14\0\0\0\0\0\0"_b;
static const bstr magic_snddat10 = "absnddat10\0\0\0\0\0\0"_b;
static const bstr magic_snddat11 = "absnddat11\0\0\0\0\0\0"_b;
static const bstr magic_data10 = "abdata10\0\0\0\0\0\0\0\0"_b;
static const bstr magic_data11 = "abdata11\0\0\0\0\0\0\0\0"_b;
static const bstr magic_data12 = "abdata12\0\0\0\0\0\0\0\0"_b;
static const bstr magic_data13 = "abdata13\0\0\0\0\0\0\0\0"_b;
static const bstr magic_image10 = "abimage10\0\0\0\0\0\0\0"_b;
static const bstr magic_sound10 = "absound10\0\0\0\0\0\0\0"_b;

static int guess_version(io::Stream &arc_stream)
{
    bstr magic = arc_stream.read(16);
    if (magic == magic10)
        return 10;
    if (magic == magic11)
        return 11;
    if (magic == magic12)
        return 12;
    return -1;
}

static void read_data_entry(File &input_file, fmt::ArchiveMeta &meta)
{
    auto entry = std::make_unique<ArchiveEntryImpl>();
    entry->name = "unknown.dat";
    entry->size = input_file.stream.read_u32_le();
    entry->offset = input_file.stream.tell();
    input_file.stream.skip(entry->size);
    meta.entries.push_back(std::move(entry));
}

static void read_resource_entry(File &input_file, fmt::ArchiveMeta &meta)
{
    bstr magic = input_file.stream.read(16);

    auto entry = std::make_unique<ArchiveEntryImpl>();
    auto name_size = input_file.stream.read_u16_le();
    entry->name = util::sjis_to_utf8(input_file.stream.read(name_size)).str();
    if (entry->name.empty())
        entry->name = "unknown";
    entry->name += ".dat";

    if (magic == magic_snddat11
        || magic == magic_imgdat11
        || magic == magic_imgdat13
        || magic == magic_imgdat14)
    {
        input_file.stream.skip(input_file.stream.read_u16_le());
    }
    else if (magic != magic_imgdat10 && magic != magic_snddat10)
    {
        throw err::NotSupportedError(util::format(
            "Unknown image magic: %s", magic.get<char>()));
    }

    input_file.stream.skip(1);
    if (magic == magic_imgdat14)
        input_file.stream.skip(76);
    if (magic == magic_imgdat13)
        input_file.stream.skip(12);

    entry->size = input_file.stream.read_u32_le();
    entry->offset = input_file.stream.tell();
    input_file.stream.skip(entry->size);
    if (entry->size)
        meta.entries.push_back(std::move(entry));
}

bool Abmp10ArchiveDecoder::is_recognized_impl(File &input_file) const
{
    return guess_version(input_file.stream) >= 0;
}

std::unique_ptr<fmt::ArchiveMeta>
    Abmp10ArchiveDecoder::read_meta_impl(File &input_file) const
{
    input_file.stream.seek(16);
    auto meta = std::make_unique<ArchiveMeta>();
    while (input_file.stream.tell() < input_file.stream.size())
    {
        auto magic = input_file.stream.read(16);
        if (magic == magic_data10
            || magic == magic_data11
            || magic == magic_data12
            || magic == magic_data13)
        {
            read_data_entry(input_file, *meta);
        }
        else if (magic == magic_image10 || magic == magic_sound10)
        {
            size_t file_count = input_file.stream.read_u8();
            for (auto i : util::range(file_count))
                read_resource_entry(input_file, *meta);
        }
        else
        {
            throw err::NotSupportedError(util::format(
                "Unknown section: %s", magic.get<char>()));
        }
    }
    return meta;
}

std::unique_ptr<File> Abmp10ArchiveDecoder::read_file_impl(
    File &input_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    input_file.stream.seek(entry->offset);
    auto data = input_file.stream.read(entry->size);
    auto output_file = std::make_unique<File>(entry->name, data);
    output_file->guess_extension();
    return output_file;
}

static auto dummy = fmt::register_fmt<Abmp10ArchiveDecoder>("qlie/abmp10");
