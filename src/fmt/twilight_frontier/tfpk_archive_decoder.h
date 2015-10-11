#pragma once

#include "fmt/archive_decoder.h"

namespace au {
namespace fmt {
namespace twilight_frontier {

    class TfpkArchiveDecoder final : public ArchiveDecoder
    {
    public:
        TfpkArchiveDecoder();
        ~TfpkArchiveDecoder();
        void register_cli_options(ArgParser &) const override;
        void parse_cli_options(const ArgParser &) override;
        std::vector<std::string> get_linked_formats() const override;
    protected:
        bool is_recognized_impl(File &) const override;
        std::unique_ptr<ArchiveMeta> read_meta_impl(File &) const override;
        std::unique_ptr<File> read_file_impl(
            File &, const ArchiveMeta &, const ArchiveEntry &) const override;
        void preprocess(
            File &, ArchiveMeta &, const FileSaver &) const override;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }
