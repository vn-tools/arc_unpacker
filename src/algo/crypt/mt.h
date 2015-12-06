#pragma once

#include <array>
#include <memory>
#include "types.h"

namespace au {
namespace algo {
namespace crypt {

    class MersenneTwister final
    {
    public:
        static std::unique_ptr<MersenneTwister> Knuth(const u32 seed);
        static std::unique_ptr<MersenneTwister> Classic(const u32 seed);
        static std::unique_ptr<MersenneTwister> Improved(const u32 seed);

        ~MersenneTwister();

        u32 next_u32();

    private:
        struct Priv;
        MersenneTwister(
            const std::function<void(Priv*, const u32)> seed_func,
            const u32 default_seed);
        std::unique_ptr<Priv> p;
    };

} } }
