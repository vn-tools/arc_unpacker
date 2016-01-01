#pragma once

#include "dec/base_archive_decoder.h"

namespace au {
namespace dec {
namespace purple_software {

    class Cpz5ArchiveDecoder final : public BaseArchiveDecoder
    {
    public:
        Cpz5ArchiveDecoder(const size_t version);
        ~Cpz5ArchiveDecoder();

        std::vector<std::string> get_linked_formats() const override;

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
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }
