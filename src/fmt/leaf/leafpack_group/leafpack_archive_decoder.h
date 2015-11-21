#pragma once

#include "fmt/archive_decoder.h"

namespace au {
namespace fmt {
namespace leaf {

    class LeafpackArchiveDecoder final : public ArchiveDecoder
    {
    public:
        LeafpackArchiveDecoder();
        ~LeafpackArchiveDecoder();
        void register_cli_options(ArgParser &arg_parser) const;
        void parse_cli_options(const ArgParser &arg_parser);
        void set_key(const bstr &key);
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
