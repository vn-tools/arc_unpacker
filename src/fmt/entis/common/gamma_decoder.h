#pragma once

#include "fmt/entis/common/decoder.h"
#include "io/bit_reader.h"

namespace au {
namespace fmt {
namespace entis {
namespace common {

    int get_gamma_code(io::BitReader &bit_reader);

    class GammaDecoder final : public Decoder
    {
    public:
        GammaDecoder();
        ~GammaDecoder();

        virtual void reset() override;
        virtual void decode(u8 *output, size_t output_size) override;

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } } }
