#pragma once

#include <memory>
#include "fmt/entis/audio/common.h"

namespace au {
namespace fmt {
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
