#pragma once

#include "dec/base_image_decoder.h"

namespace au {
namespace dec {
namespace png {

    class PngImageDecoder final : public BaseImageDecoder
    {
    public:
        using ChunkHandler = std::function<void(
            const std::string &chunk_name, const bstr &chunk_data)>;

        using BaseImageDecoder::decode;
        res::Image decode(
            const Logger &logger,
            io::File &input_file,
            ChunkHandler chunk_handler) const;

    protected:
        bool is_recognized_impl(io::File &input_file) const override;
        res::Image decode_impl(
            const Logger &logger, io::File &input_file) const override;
    };

} } }
