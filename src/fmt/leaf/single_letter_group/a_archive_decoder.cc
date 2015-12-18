#include "fmt/leaf/single_letter_group/a_archive_decoder.h"
#include "algo/locale.h"
#include "algo/pack/lzss.h"
#include "algo/range.h"
#include "err.h"
#include "io/memory_stream.h"

using namespace au;
using namespace au::fmt::leaf;

static const bstr magic = "\x1E\xAF"_b; // LEAF in hexspeak

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
        u8 flags;
    };
}

static bstr decrypt_1(const bstr &input)
{
    throw err::NotSupportedError("Type 1 encryption is not supported");
}

static bstr decrypt_2(const bstr &input)
{
    throw err::NotSupportedError("Type 2 encryption is not supported");
}

static bstr decrypt_3(const bstr &input, const u8 key)
{
    if (input.size() < 0x20)
        return input;

    bstr output(input);

    io::MemoryStream input_stream(input);
    input_stream.seek(0);
    const auto width = input_stream.read_u32_le();
    const auto height = input_stream.read_u32_le();
    const auto size = width * height;
    if (32 + size * 4 > input_stream.size())
        return input;

    s8 acc[3] = {0, 0, 0};
    size_t output_pos = 32;
    for (const auto i : algo::range(size))
    {
        s8 *output_ptr = output.get<s8>() + output_pos;
        const auto tmp = output_ptr[3];
        acc[0] += tmp + output_ptr[0] - key;
        acc[1] += tmp + output_ptr[1] - key;
        acc[2] += tmp + output_ptr[2] - key;
        output_ptr[0] = acc[0];
        output_ptr[1] = acc[1];
        output_ptr[2] = acc[2];
        output_ptr[3] = 0;
        output_pos += 4;
    }

    return output;
}

bool AArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    AArchiveDecoder::read_meta_impl(io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto file_count = input_file.stream.read_u16_le();
    const auto offset_to_data = input_file.stream.tell() + 32 * file_count;
    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->path = input_file.stream.read_to_zero(23).str();
        entry->flags = input_file.stream.read_u8();
        entry->size = input_file.stream.read_u32_le();
        entry->offset = input_file.stream.read_u32_le() + offset_to_data;
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> AArchiveDecoder::read_file_impl(
    io::File &input_file,
    const fmt::ArchiveMeta &m,
    const fmt::ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    input_file.stream.seek(entry->offset);

    bstr data;
    if (entry->flags)
    {
        const auto size_orig = input_file.stream.read_u32_le();
        data = input_file.stream.read(entry->size-4);
        data = algo::pack::lzss_decompress_bytewise(data, size_orig);

        // this "encryption" apparently concerns only gfx
        if (entry->flags == 3)
            data = decrypt_1(data);
        else if (entry->flags == 5)
            data = decrypt_2(data);
        else if (entry->flags >= 0x7F && entry->flags <= 0x89)
            data = decrypt_3(data, entry->flags & 0x0F);
    }
    else
    {
        data = input_file.stream.read(entry->size);
    }
    auto output_file = std::make_unique<io::File>(entry->path, data);
    output_file->guess_extension();
    return output_file;
}

std::vector<std::string> AArchiveDecoder::get_linked_formats() const
{
    return {"leaf/w", "leaf/px"};
}

static auto dummy = fmt::register_fmt<AArchiveDecoder>("leaf/a");
