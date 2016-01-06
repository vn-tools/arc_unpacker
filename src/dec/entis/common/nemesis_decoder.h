#pragma once

#include "dec/entis/common/base_decoder.h"
#include "dec/entis/common/prob_model.h"

namespace au {
namespace dec {
namespace entis {
namespace common {

    class NemesisDecoder final : public BaseDecoder
    {
    public:
        NemesisDecoder();
        ~NemesisDecoder();

        void reset() override;
        void decode(u8 *output, const size_t output_size) override;

        int decode_erisa_code(ProbModel &model);
        int decode_erisa_code_index(const ProbModel &model);

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } } }
