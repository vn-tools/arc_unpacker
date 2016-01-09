#pragma once

#include "dec/entis/common/base_erisa_decoder.h"
#include "dec/entis/common/prob_model.h"

namespace au {
namespace dec {
namespace entis {
namespace common {

    class ErisaNDecoder final : public BaseErisaDecoder
    {
    public:
        ErisaNDecoder();
        ~ErisaNDecoder();

        void reset() override;
        void decode(u8 *ouptut, const size_t output_size) override;

    protected:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } } }
