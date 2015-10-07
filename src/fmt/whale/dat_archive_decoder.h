#pragma once

#include "fmt/archive_decoder.h"

namespace au {
namespace fmt {
namespace whale {

    class DatArchiveDecoder final : public ArchiveDecoder
    {
    public:
        DatArchiveDecoder();
        ~DatArchiveDecoder();
        void set_game_title(const std::string &game_title);
        void add_file_name(const std::string &file_name);
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
