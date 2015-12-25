#pragma once

#include "fmt/archive_decoder.h"
#include "res/image.h"

namespace au {
namespace fmt {
namespace will {

    class WipfImageArchiveDecoder final : public ArchiveDecoder
    {
    public:
        std::vector<std::shared_ptr<res::Image>> unpack_to_images(
            const Logger &logger,
            io::File &input_file) const;

        std::unique_ptr<res::Image> read_image(
            const Logger &logger,
            io::File &input_file,
            const ArchiveMeta &m,
            const ArchiveEntry &e) const;

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

        NamingStrategy naming_strategy() const override;
    };

} } }
