#pragma once

#include "dec/base_archive_decoder.h"

namespace au {
namespace dec {
namespace kaguya {

    class BaseLinkArchiveDecoder : public BaseArchiveDecoder
    {
    public:
        virtual ~BaseLinkArchiveDecoder() {}
        std::vector<std::string> get_linked_formats() const override;

    protected:
        std::unique_ptr<ArchiveMeta> read_meta_impl(
            const Logger &logger,
            io::File &input_file) const override;

        std::unique_ptr<io::File> read_file_impl(
            const Logger &logger,
            io::File &input_file,
            const ArchiveMeta &m,
            const ArchiveEntry &e) const override;

        virtual int get_version() const = 0;
    };

} } }
