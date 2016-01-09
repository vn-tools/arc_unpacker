#pragma once

#include <boost/optional.hpp>
#include "dec/base_archive_decoder.h"

namespace au {
namespace dec {
namespace leaf {

    class Pak1ArchiveDecoder final : public BaseArchiveDecoder
    {
    public:
        Pak1ArchiveDecoder();
        std::vector<std::string> get_linked_formats() const override;
        void set_version(const int version);

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
        boost::optional<int> version;
    };

} } }
