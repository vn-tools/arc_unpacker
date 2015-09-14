#pragma once

#include "fmt/entis/common/decoder.h"
#include "fmt/entis/common/prob_model.h"

namespace au {
namespace fmt {
namespace entis {
namespace common {

    class NemesisDecoder final : public Decoder
    {
    public:
        NemesisDecoder();
        ~NemesisDecoder();

        virtual void reset() override;
        virtual void decode(u8 *output, size_t output_size) override;

        int decode_erisa_code(ProbModel &model);
        int decode_erisa_code_index(const ProbModel &model);

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } } }
