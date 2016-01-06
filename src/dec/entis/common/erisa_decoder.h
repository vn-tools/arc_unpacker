#pragma once

#include "dec/entis/common/base_erisa_decoder.h"
#include "dec/entis/common/prob_model.h"

namespace au {
namespace dec {
namespace entis {
namespace common {

    class ErisaDecoder final : public BaseErisaDecoder
    {
    public:
        ErisaDecoder();
        ~ErisaDecoder();

        void reset() override;
        void decode(u8 *output, const size_t output_size) override;

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } } }
