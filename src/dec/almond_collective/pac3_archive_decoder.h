#pragma once

#include "dec/base_archive_decoder.h"

namespace au {
namespace dec {
namespace almond_collective {

    class Pac3ArchiveDecoder final : public BaseArchiveDecoder
    {
    public:
        Pac3ArchiveDecoder();

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

    private:
        std::string game_key;
    };

} } }
