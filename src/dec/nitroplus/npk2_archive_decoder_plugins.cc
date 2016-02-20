#include "dec/nitroplus/npk2_archive_decoder.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::nitroplus;

Npk2ArchiveDecoder::Npk2ArchiveDecoder()
{
    plugin_manager.add(
        "tokyo-necro",
        "Tokyo Necro",
        "\x96\x2C\x5F\x3A\x78\x9C\x84\x37"
        "\xB7\x12\x12\xA1\x15\xD6\xCA\x9F"
        "\x9A\xE3\xFD\x21\x0F\xF6\xAF\x70"
        "\xA8\xA8\xF8\xBB\xFE\x5E\x8A\xF5"_b);

    add_arg_parser_decorator(
        plugin_manager.create_arg_parser_decorator(
            "Selects NPK2 decryption routine."));
}
