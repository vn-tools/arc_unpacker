#include "dec/malie/libu_archive_decoder.h"
#include "dec/malie/common/lib_plugins.h"

using namespace au;
using namespace au::dec::malie;

LibuArchiveDecoder::LibuArchiveDecoder()
{
    common::add_common_lib_plugins(plugin_manager);

    add_arg_parser_decorator(
        plugin_manager.create_arg_parser_decorator(
            "Selects Camellia decryption key."));
}
