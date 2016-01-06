#pragma once

#include "dec/entis/common/base_decoder.h"
#include "dec/entis/common/prob_model.h"

namespace au {
namespace dec {
namespace entis {
namespace common {

    class BaseErisaDecoder : public BaseDecoder
    {
    public:
        int decode_erisa_code(ProbModel &model);
        int decode_erisa_code_index(const ProbModel &model);

    protected:
        u32 code_register;
        u32 augend_register;
    };

} } } }
