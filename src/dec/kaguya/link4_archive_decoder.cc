#include "dec/kaguya/link4_archive_decoder.h"
#include "algo/locale.h"
#include "algo/range.h"
#include "dec/kaguya/common/params_encryption.h"
#include "err.h"
#include "virtual_file_system.h"

using namespace au;
using namespace au::dec::kaguya;

static const auto magic = "LINK4"_b;

namespace
{
    struct ArchiveMetaImpl final : dec::ArchiveMeta
    {
        bstr key;
    };

    struct ArchiveEntryImpl final : dec::ArchiveEntry
    {
        size_t offset;
        size_t size;
        bool encrypted;
    };
}

bool Link4ArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> Link4ArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(8);
    auto meta = std::make_unique<ArchiveMetaImpl>();
    auto params_file = VirtualFileSystem::get_by_name("params.dat");
    if (params_file)
    {
        try
        {
            meta->key = common::get_key_from_params_file(params_file->stream);
        }
        catch (const std::exception &e)
        {
            logger.warn("%s\n", e.what());
        }
    }

    input_file.stream.skip(2);
    while (true)
    {
        const auto entry_offset = input_file.stream.tell();
        const auto entry_size = input_file.stream.read_le<u32>();
        if (!entry_size)
            break;

        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->encrypted = input_file.stream.read<u8>() & 4;
        input_file.stream.skip(8);

        const auto entry_name_size = input_file.stream.read<u8>();
        input_file.stream.skip(2);
        entry->path = algo::sjis_to_utf8(
            input_file.stream.read(entry_name_size)).str();
        entry->offset = input_file.stream.tell();
        entry->size = entry_size - (entry->offset - entry_offset);
        meta->entries.push_back(std::move(entry));

        input_file.stream.seek(entry_offset + entry_size);
    }

    return std::move(meta);
}

std::unique_ptr<io::File> Link4ArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto meta = static_cast<const ArchiveMetaImpl*>(&m);
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    auto data = input_file.stream.seek(entry->offset).read(entry->size);
    if (entry->encrypted)
    {
        if (meta->key.empty())
            throw err::CorruptDataError("Missing decryption key");
        if (meta->key.size() < 0x3A980)
            throw err::CorruptDataError("Corrupt decryption key");
        const auto offset = common::get_encryption_offset(data);
        if (offset != -1)
            for (const auto i : algo::range(offset, data.size()))
                data[i] ^= meta->key[(i - offset) % 0x3A980];
    }
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> Link4ArchiveDecoder::get_linked_formats() const
{
    return
    {
        "kaguya/ap",
        "kaguya/ap0",
        "kaguya/ap2",
        "kaguya/anm",
        "kaguya/bmr",
        "microsoft/bmp"
    };
}

static auto _ = dec::register_decoder<Link4ArchiveDecoder>("kaguya/link4");
