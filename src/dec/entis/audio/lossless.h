#pragma once

#include <memory>
#include "dec/entis/audio/common.h"

namespace au {
namespace dec {
namespace entis {
namespace audio {

    class LosslessAudioDecoder final : public BaseAudioDecoder
    {
    public:
        LosslessAudioDecoder(const MioHeader &header);
        ~LosslessAudioDecoder();

        bstr process_chunk(const MioChunk &chunk) override;

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } } }
