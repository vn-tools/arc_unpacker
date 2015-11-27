#pragma once

#include "fmt/image_decoder.h"

namespace au {
namespace fmt {
namespace png {

    class PngImageDecoder final : public ImageDecoder
    {
    public:
        using ChunkHandler = std::function<void(
            const std::string &chunk_name, const bstr &chunk_data)>;

        using ImageDecoder::decode;
        res::Image decode(
            io::File &input_file, ChunkHandler chunk_handler) const;

    protected:
        bool is_recognized_impl(io::File &input_file) const override;
        res::Image decode_impl(io::File &input_file) const override;
    };

} } }
