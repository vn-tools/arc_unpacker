#pragma once

#include "fmt/archive_decoder.h"

namespace au {
namespace fmt {
namespace qlie {

    class PackArchiveDecoder final : public ArchiveDecoder
    {
    public:
        PackArchiveDecoder();
        ~PackArchiveDecoder();
        void register_cli_options(ArgParser &) const override;
        void parse_cli_options(const ArgParser &) override;
        std::vector<std::string> get_linked_formats() const override;
    protected:
        bool is_recognized_impl(File &) const override;
        std::unique_ptr<ArchiveMeta> read_meta_impl(File &) const override;
        std::unique_ptr<File> read_file_impl(
            File &, const ArchiveMeta &, const ArchiveEntry &) const override;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }
