#pragma once

#include "dec/base_archive_decoder.h"

namespace au {
namespace dec {
namespace rpgmaker {
namespace rgs {

    struct ArchiveEntryImpl final : dec::ArchiveEntry
    {
        size_t offset;
        size_t size;
        u32 key;
    };

    u32 advance_key(const u32 key);

    std::unique_ptr<io::File> read_file_impl(
        io::File &input_file,
        const ArchiveEntryImpl &entry);

} } } }
