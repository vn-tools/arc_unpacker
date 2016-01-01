#pragma once

#include <memory>
#include "types.h"

namespace au {
namespace dec {
namespace qlie {

    class CustomMersenneTwister final
    {
    public:
        CustomMersenneTwister(u32 seed);
        ~CustomMersenneTwister();

        void xor_state(const bstr &data);
        u32 get_next_integer();

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }
