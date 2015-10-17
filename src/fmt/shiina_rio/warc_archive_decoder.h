#pragma once

#include "fmt/archive_decoder.h"

namespace au {
namespace fmt {
namespace shiina_rio {

    class WarcArchiveDecoder final : public ArchiveDecoder
    {
    public:
        WarcArchiveDecoder();
        ~WarcArchiveDecoder();
        void register_cli_options(ArgParser &arg_parser) const override;
        void parse_cli_options(const ArgParser &arg_parser) override;
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
