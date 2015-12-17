#pragma once

#include "fmt/archive_decoder.h"

namespace au {
namespace fmt {
namespace purple_software {

    class Cpz5ArchiveDecoder final : public ArchiveDecoder
    {
    public:
        Cpz5ArchiveDecoder(const size_t version);
        ~Cpz5ArchiveDecoder();

        std::vector<std::string> get_linked_formats() const override;

    protected:
        bool is_recognized_impl(io::File &input_file) const override;

        std::unique_ptr<ArchiveMeta> read_meta_impl(
            io::File &input_file) const override;

        std::unique_ptr<io::File> read_file_impl(
            io::File &input_file,
            const ArchiveMeta &m,
            const ArchiveEntry &e) const override;

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }
