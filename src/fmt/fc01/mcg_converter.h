#pragma once

#include "fmt/converter.h"

namespace au {
namespace fmt {
namespace fc01 {

    class McgConverter final : public Converter
    {
    public:
        McgConverter();
        ~McgConverter();
        void register_cli_options(ArgParser &arg_parser) const;
        void parse_cli_options(const ArgParser &arg_parser);
        void set_key(u8 key);
    protected:
        bool is_recognized_internal(File &) const override;
        std::unique_ptr<File> decode_internal(File &) const override;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }
