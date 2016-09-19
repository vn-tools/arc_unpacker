#pragma once

#include "dec/cyberworks/dat_plugin.h"
#include "plugin_manager.h"

namespace au {
namespace dec {
namespace cyberworks {
namespace common {

    void register_plugins(PluginManager<DatPlugin> &plugin_manager);

} } } }
