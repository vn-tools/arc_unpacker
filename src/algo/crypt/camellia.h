#pragma once

#include <vector>
#include "types.h"

namespace au {
namespace algo {
namespace crypt {

    class Camellia final
    {
    public:
        Camellia(
            const std::vector<u32> &key,
            const size_t grand_rounds = 3);

        void encrypt_block_128(
            const size_t block_offset,
            const u32 input[4],
            u32 output[4]) const;

        void decrypt_block_128(
            const size_t block_offset,
            const u32 input[4],
            u32 output[4]) const;

    private:
        const std::vector<u32> key;
        const size_t grand_rounds;
    };

} } }
