#pragma once

#include "fmt/image_decoder.h"

namespace au {
namespace fmt {
namespace fc01 {

    class McgImageDecoder final : public ImageDecoder
    {
    public:
        McgImageDecoder();
        ~McgImageDecoder();
        void register_cli_options(ArgParser &arg_parser) const;
        void parse_cli_options(const ArgParser &arg_parser);
        void set_key(u8 key);
    protected:
        bool is_recognized_internal(File &) const override;
        pix::Grid decode_internal(File &) const override;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }
