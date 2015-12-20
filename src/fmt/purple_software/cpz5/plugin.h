#pragma once

#include <array>
#include <memory>
#include "types.h"

namespace au {
namespace fmt {
namespace purple_software {
namespace cpz5 {

    struct Plugin final
    {
        bstr secret;

        std::array<size_t, 4> hash_permutation;
        std::array<u32, 4> hash_iv;
        std::array<u32, 4> hash_xor;
        std::array<u32, 4> hash_add;

        u32 crypt_23_mul;

        struct
        {
            u32 add1;
            u32 add2;
            u8 tail_sub;
        } crypt_1a;

        struct
        {
            std::array<u32, 4> addends;
            u32 init_key;
        } crypt_1c;

        struct
        {
            size_t start_pos;
            u8 tail_xor;
        } crypt_3;
    };

    std::vector<std::shared_ptr<cpz5::Plugin>> get_cpz5_plugins();
    std::vector<std::shared_ptr<cpz5::Plugin>> get_cpz6_plugins();

} } } }
