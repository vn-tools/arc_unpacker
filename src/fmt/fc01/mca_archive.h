#pragma once

#include "fmt/archive.h"

namespace au {
namespace fmt {
namespace fc01 {

    class McaArchive final : public Archive
    {
    public:
        McaArchive();
        ~McaArchive();
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
