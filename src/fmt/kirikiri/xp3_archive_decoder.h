#pragma once

#include "fmt/archive_decoder.h"

namespace au {
namespace fmt {
namespace kirikiri {

    class Xp3ArchiveDecoder final : public ArchiveDecoder
    {
    public:
        Xp3ArchiveDecoder();
        ~Xp3ArchiveDecoder();
        void register_cli_options(ArgParser &) const override;
        void parse_cli_options(const ArgParser &) override;
        std::unique_ptr<ArchiveMeta> read_meta(File &) const override;
        std::unique_ptr<File> read_file(
            File &, const ArchiveMeta &, const ArchiveEntry &) const override;
    protected:
        bool is_recognized_internal(File &) const override;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }
