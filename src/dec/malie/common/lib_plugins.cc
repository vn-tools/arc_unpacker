#include "dec/malie/common/lib_plugins.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::malie;

std::vector<u32> common::convert_decryption_key_to_encryption_key(
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

    plugin_manager.add(
        "paradise-lost",
        "Paradise Lost",
        convert_decryption_key_to_encryption_key({
            0x745A5511, 0xFC188446, 0x6729696A, 0xB7C80D5E,
            0x9AA598A4, 0xB8331AAC, 0xBD2B2BA7, 0x28A3BC2C,
            0x06AF3A2D, 0x2A88FE0C, 0x42233394, 0xB4B55BE4,
            0xDE164D52, 0xCC525C19, 0x8D565E95, 0x95D39451,
            0xCA28EF0B, 0x26A96629, 0x2E0CC6AB, 0x2F4ACAE9,
            0x2D2D56F9, 0x01ABCE8B, 0x4AA23F83, 0x1088CCE5,
            0x99CA5A5A, 0xADF20357, 0xB3149706, 0x635597A5,
            0x2F4ACAE9, 0xCA28EF0B, 0x26A96629, 0x2E0CC6AB,
            0x42233394, 0xB4B55BE4, 0x06AF3A2D, 0x2A88FE0C,
            0xFC188446, 0x6729696A, 0xB7C80D5E, 0x745A5511,
            0xB8331AAC, 0xBD2B2BA7, 0x28A3BC2C, 0x9AA598A4,
            0xAA23F831, 0x088CCE52, 0xD2D56F90, 0x1ABCE8B4,
            0x31497066, 0x35597A56, 0x574E5147, 0x7859354B,
        }));
}
