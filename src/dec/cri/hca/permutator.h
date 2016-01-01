#pragma once

#include <array>
#include <memory>
#include "types.h"

namespace au {
namespace dec {
namespace cri {
namespace hca {

    class Permutator final
    {
    public:
        Permutator(const u16 type, const u32 key1, const u32 key2);
        ~Permutator();
        bstr permute(const bstr &data);

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } } }
