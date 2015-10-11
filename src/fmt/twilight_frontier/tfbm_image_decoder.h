#pragma once

#include "fmt/image_decoder.h"

namespace au {
namespace fmt {
namespace twilight_frontier {

    class TfbmImageDecoder final : public ImageDecoder
    {
    public:
        TfbmImageDecoder();
        ~TfbmImageDecoder();
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
