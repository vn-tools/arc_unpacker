#pragma once

#include "plugin_manager.h"
#include "types.h"

namespace au {
namespace dec {
namespace malie {
namespace common {

    void add_common_lib_plugins(
        PluginManager<std::vector<u32>> &plugin_manager);

} } } }
