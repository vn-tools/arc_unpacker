#pragma once

#include "fmt/entis/audio/common.h"
#include "fmt/entis/common/decoder.h"

namespace au {
namespace fmt {
namespace entis {
namespace audio {

    class LossyAudioDecoder final : public AudioDecoderImpl
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
