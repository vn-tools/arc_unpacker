#pragma once

#include "fmt/image_decoder.h"

namespace au {
namespace fmt {
namespace png {

    class PngImageDecoder final : public ImageDecoder
    {
    public:
        using ChunkHandler
            = std::function<void(const std::string &, const bstr &)>;

        using ImageDecoder::decode;
        pix::Grid decode(File &, ChunkHandler chunk_handler) const;

    protected:
        bool is_recognized_impl(File &) const override;
        pix::Grid decode_impl(File &) const override;
    };

} } }
