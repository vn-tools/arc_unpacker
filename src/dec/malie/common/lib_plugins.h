#pragma once

#include "plugin_manager.h"
#include "types.h"

namespace au {
namespace dec {
namespace malie {
namespace common {

    std::vector<u32> convert_decryption_key_to_encryption_key(
        const std::vector<u32> &input);

    void add_common_lib_plugins(
        PluginManager<std::vector<u32>> &plugin_manager);

} } } }
