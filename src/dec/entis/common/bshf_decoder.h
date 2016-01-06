#pragma once

#include "dec/entis/common/base_decoder.h"
#include "types.h"

namespace au {
namespace dec {
namespace entis {
namespace common {

    class BshfDecoder final : public BaseDecoder
    {
    public:
        BshfDecoder(const bstr &key);
        ~BshfDecoder();
        void reset() override;
        void decode(u8 *ouptut, const size_t output_size) override;

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } } }
