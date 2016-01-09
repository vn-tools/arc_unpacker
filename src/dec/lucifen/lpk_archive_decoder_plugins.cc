#include "dec/lucifen/lpk_archive_decoder.h"

using namespace au;
using namespace au::dec::lucifen;

LpkArchiveDecoder::LpkArchiveDecoder()
{
    plugin_manager.add(
        "sakura-sync",
        "Sakura Synchronicity",
        {
            {0xA5B9AC6B, 0x9A639DE5},
            0x5D,
            0x31746285,
            {
                {"script",  {0x00000000, 0x00000000}},
                {"sys",     {0xAE91B6B5, 0x9D42ED5C}},
                {"chr",     {0x7DC8994E, 0xB6E42499}},
                {"pic",     {0xC69D1636, 0x387DB369}},
                {"bgm",     {0x8C79B285, 0xBAE4AE69}},
                {"se",      {0x453B8E8B, 0x84C15E88}},
                {"voice",   {0x936E96DA, 0x5B7388E8}},
                {"data",    {0x2D4DAAC8, 0xE15D75AE}},
            }
        });

    add_arg_parser_decorator(
        plugin_manager.create_arg_parser_decorator(
            "Selects LPK decryption routine."));
}
