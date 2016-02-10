#pragma once

#include "io/base_byte_stream.h"

namespace au {
namespace dec {
namespace kaguya {
namespace common {

    bstr get_key_from_params_file(io::BaseByteStream &input_stream);

    int get_encryption_offset(const bstr &data);

} } } }
