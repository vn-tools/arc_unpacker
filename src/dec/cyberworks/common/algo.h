#pragma once

#include "dec/cyberworks/dat_plugin.h"
#include "io/memory_byte_stream.h"
#include "logger.h"

namespace au {
namespace dec {
namespace cyberworks {
namespace common {

    void decode_data(
        const Logger &logger,
        const bstr &type,
        bstr &data,
        const DatPlugin &plugin);

    u32 read_obfuscated_number(io::BaseByteStream &input_stream);

} } } }
