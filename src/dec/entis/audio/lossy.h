#pragma once

#include <memory>
#include "dec/entis/audio/common.h"

namespace au {
namespace dec {
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
