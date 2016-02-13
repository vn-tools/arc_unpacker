#pragma once

#include "dec/base_archive_decoder.h"

namespace au {
namespace dec {
namespace rpgmaker {
namespace rgs {

    struct CustomArchiveEntry final : dec::PlainArchiveEntry
    {
        u32 key;
    };

    u32 advance_key(const u32 key);

    std::unique_ptr<io::File> read_file_impl(
        io::File &input_file,
        const CustomArchiveEntry &entry);

} } } }
