#include "dec/malie/common/lib_plugins.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::malie;

static std::vector<u32> convert_decryption_key_to_encryption_key(
    const std::vector<u32> &input)
{
    std::vector<u32> output(input.size());
    for (const auto i : algo::range(0, input.size() & ~3, 4))
    {
        output.at(input.size() - i - 4) = input.at(i + 0);
        output.at(input.size() - i - 3) = input.at(i + 1);
        output.at(input.size() - i - 2) = input.at(i + 2);
        output.at(input.size() - i - 1) = input.at(i + 3);
    }
    return output;
}

void common::add_common_lib_plugins(
    PluginManager<std::vector<u32>> &plugin_manager)
{
    plugin_manager.add("noop", "Unencrypted", {});

    plugin_manager.add(
        "dies-irae",
        "Dies Irae",
        convert_decryption_key_to_encryption_key({
            0x6F388B64, 0xBB5B3676, 0x2317DD18, 0x7CCD3736,
            0x00000000, 0x00000000, 0x00000000, 0x00000000,
            0x9B9B379C, 0x45B25DAD, 0x9B3B118B, 0xEE8C3E66,
            0x00000000, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0x00000000, 0x00000000,
            0xFBA30F99, 0xA6E6CDE7, 0x116C976B, 0x66CEC462,
            0x88C5F746, 0x1F334DCD, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0x00000000, 0x00000000,
            0x9B3B118B, 0xEE8C3E66, 0x9B9B379C, 0x45B25DAD,
            0xBB5B3676, 0x2317DD18, 0x7CCD3736, 0x6F388B64,
            0x00000000, 0x00000000, 0x00000000, 0x00000000,
            0x16C976B6, 0x6CEC462F, 0xBA30F99A, 0x6E6CDE71,
            0x00000000, 0x00000000, 0x00000000, 0x00000000,
        }));
}
