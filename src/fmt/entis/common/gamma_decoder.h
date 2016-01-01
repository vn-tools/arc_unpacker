#pragma once

#include "fmt/entis/common/abstract_decoder.h"
#include "io/ibit_reader.h"

namespace au {
namespace fmt {
namespace entis {
namespace common {

    int get_gamma_code(io::IBitReader &bit_reader);

    class GammaDecoder final : public AbstractDecoder
    {
    public:
        GammaDecoder();
        ~GammaDecoder();

        void reset() override;
        void decode(u8 *output, size_t output_size) override;

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } } }
