#pragma once

#include "fmt/entis/common/decoder.h"
#include "fmt/entis/sound/common.h"

namespace au {
namespace fmt {
namespace entis {
namespace sound {

    class LossySoundDecoder final : public SoundDecoderImpl
    {
    public:
        LossySoundDecoder(const MioHeader &header);
        ~LossySoundDecoder();

        virtual bstr process_chunk(const MioChunk &chunk) override;

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } } }
