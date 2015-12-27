#pragma once

#include <memory>
#include "fmt/entis/audio/common.h"

namespace au {
namespace fmt {
namespace entis {
namespace audio {

    class LossyAudioDecoder final : public BaseAudioDecoder
    {
    public:
        LossyAudioDecoder(const MioHeader &header);
        ~LossyAudioDecoder();

        bstr process_chunk(const MioChunk &chunk) override;

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } } }
