#pragma once

#include "fmt/archive_decoder.h"

namespace au {
namespace fmt {
namespace team_shanghai_alice {

    class ThbgmAudioArchiveDecoder final : public ArchiveDecoder
    {
    public:
        ThbgmAudioArchiveDecoder();
        ~ThbgmAudioArchiveDecoder();
        bool is_recognized_impl(File &) const override;
        void register_cli_options(ArgParser &arg_parser) const override;
        void parse_cli_options(const ArgParser &arg_parser) override;
        void set_loop_count(size_t loop_count);
        std::unique_ptr<ArchiveMeta> read_meta_impl(File &) const override;
        std::unique_ptr<File> read_file_impl(
            File &, const ArchiveMeta &, const ArchiveEntry &) const override;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }
