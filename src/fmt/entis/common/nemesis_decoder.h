#pragma once

#include "fmt/entis/common/abstract_decoder.h"
#include "fmt/entis/common/prob_model.h"

namespace au {
namespace fmt {
namespace entis {
namespace common {

    class NemesisDecoder final : public AbstractDecoder
    {
    public:
        NemesisDecoder();
        ~NemesisDecoder();

        void reset() override;
        void decode(u8 *output, size_t output_size) override;

        int decode_erisa_code(ProbModel &model);
        int decode_erisa_code_index(const ProbModel &model);

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } } }
