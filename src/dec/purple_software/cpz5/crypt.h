#pragma once

#include <array>
#include "algo/ptr.h"
#include "dec/purple_software/cpz5/plugin.h"

namespace au {
namespace dec {
namespace purple_software {
namespace cpz5 {

    std::array<u32, 4> get_hash(
        const Plugin &plugin, const std::array<u32, 4> &input_dwords);

    void decrypt_1a(
        algo::ptr<u8> target, const Plugin &plugin, const u32 key);

    void decrypt_1b(
        algo::ptr<u8> target, const u32 key, const std::array<u32, 4> &hash);

    void decrypt_1c(
        algo::ptr<u8> target,
        const Plugin &plugin,
        const u32 key,
        const std::array<u32, 4> &hash);

    void decrypt_2(
        algo::ptr<u8> target,
        const Plugin &plugin,
        const u32 key,
        const u32 seed,
        const u8 permutation_xor);

    void decrypt_3(
        algo::ptr<u8> target,
        const Plugin &plugin,
        const u32 key,
        const u32 seed,
        const std::array<u32, 4> &hash,
        const u32 entry_key);

} } } }
