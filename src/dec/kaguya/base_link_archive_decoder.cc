#include "dec/kaguya/base_link_archive_decoder.h"
#include "algo/binary.h"
#include "algo/locale.h"
#include "algo/range.h"
#include "dec/kaguya/common/params_encryption.h"
#include "err.h"
#include "virtual_file_system.h"

using namespace au;
using namespace au::dec::kaguya;

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

std::unique_ptr<dec::ArchiveMeta> BaseLinkArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
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

    if (get_version() == 3)
    {
        input_file.stream.seek(8);
    }
    else if (get_version() == 4 || get_version() == 5)
    {
        input_file.stream.seek(10);
    }
    else if (get_version() == 6)
    {
        input_file.stream.seek(7);
        const auto name_size = input_file.stream.read<u8>();
        const auto name = input_file.stream.read(name_size);
    }
    else
    {
        throw std::logic_error("Bad version");
    }

    while (true)
    {
        const auto entry_offset = input_file.stream.tell();
        const auto entry_size = input_file.stream.read_le<u32>();
        if (!entry_size)
            break;

        auto entry = std::make_unique<ArchiveEntryImpl>();
        const auto flags = input_file.stream.read_le<u16>();
        entry->encrypted = flags & 4;
        input_file.stream.skip(7);

        if (get_version() == 3 || get_version() == 4 || get_version() == 5)
        {
            const auto name_size = input_file.stream.read<u8>();
            input_file.stream.skip(2);
            entry->path = algo::sjis_to_utf8(
                input_file.stream.read(name_size)).str();
        }
        else if (get_version() == 6)
        {
            const auto name_size = input_file.stream.read_le<u16>();
            entry->path = algo::utf16_to_utf8(
                input_file.stream.read(name_size)).str();
        }

        entry->offset = input_file.stream.tell();
        entry->size = entry_size - (entry->offset - entry_offset);
        meta->entries.push_back(std::move(entry));

        input_file.stream.seek(entry_offset + entry_size);
    }

    return meta;
}

std::unique_ptr<io::File> BaseLinkArchiveDecoder::read_file_impl(
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

std::vector<std::string> BaseLinkArchiveDecoder::get_linked_formats() const
{
    return
    {
        "kaguya/ap",
        "kaguya/ap0",
        "kaguya/ap2",
        "kaguya/ap3",
        "kaguya/anm",
        "kaguya/bmr",
        "microsoft/bmp"
    };
}
