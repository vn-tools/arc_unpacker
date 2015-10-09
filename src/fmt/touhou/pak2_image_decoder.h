#pragma once

#include <string>
#include "fmt/image_decoder.h"

namespace au {
namespace fmt {
namespace touhou {

    class Pak2ImageDecoder final : public ImageDecoder
    {
    public:
        Pak2ImageDecoder();
        ~Pak2ImageDecoder();
        void clear_palettes();
        void add_palette(const std::string &name, const bstr &palette_data);
    protected:
        bool is_recognized_impl(File &) const override;
        pix::Grid decode_impl(File &) const override;
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }
