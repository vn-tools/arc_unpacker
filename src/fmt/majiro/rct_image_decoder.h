#pragma once

#include "fmt/image_decoder.h"

namespace au {
namespace fmt {
namespace majiro {

    class RctImageDecoder final : public ImageDecoder
    {
    public:
        RctImageDecoder();
        ~RctImageDecoder();
        void register_cli_options(ArgParser &arg_parser) const;
        void parse_cli_options(const ArgParser &arg_parser);
        void set_key(const bstr &key);

    protected:
        bool is_recognized_impl(io::File &input_file) const override;
        pix::Grid decode_impl(io::File &input_file) const override;

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }
