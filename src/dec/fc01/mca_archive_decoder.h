#pragma once

#include <boost/optional.hpp>
#include "dec/base_archive_decoder.h"

namespace au {
namespace dec {
namespace fc01 {

    class McaArchiveDecoder final : public BaseArchiveDecoder
    {
    public:
        McaArchiveDecoder();

    protected:
        bool is_recognized_impl(io::File &input_file) const override;

        std::unique_ptr<ArchiveMeta> read_meta_impl(
            const Logger &logger,
            io::File &input_file) const override;

        std::unique_ptr<io::File> read_file_impl(
            const Logger &logger,
            io::File &input_file,
            const ArchiveMeta &m,
            const ArchiveEntry &e) const override;

    public:
        boost::optional<u8> key;
    };

} } }
