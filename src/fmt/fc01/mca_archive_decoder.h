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
        bool is_recognized_internal(File &) const override;
        void unpack_internal(File &, FileSaver &) const override;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }
