#pragma once

#include "fmt/archive_decoder.h"

namespace au {
namespace fmt {
namespace ivory {

    class MblArchiveDecoder final : public ArchiveDecoder
    {
    public:
        MblArchiveDecoder();
        ~MblArchiveDecoder();
        void register_cli_options(ArgParser &) const override;
        void parse_cli_options(const ArgParser &) override;
        void set_plugin(const std::string &name);
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
