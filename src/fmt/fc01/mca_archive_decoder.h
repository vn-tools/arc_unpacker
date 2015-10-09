#pragma once

#include "fmt/archive_decoder.h"

namespace au {
namespace fmt {
namespace fc01 {

    class McaArchiveDecoder final : public ArchiveDecoder
    {
    public:
        McaArchiveDecoder();
        ~McaArchiveDecoder();
        void register_cli_options(ArgParser &arg_parser) const;
        void parse_cli_options(const ArgParser &arg_parser);
        void set_key(u8 key);
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
