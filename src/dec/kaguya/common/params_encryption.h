#pragma once

#include "io/base_byte_stream.h"

namespace au {
namespace dec {
namespace kaguya {
namespace common {

    struct Params final
    {
        bool decrypt_anm;
        bstr game_title;
        bstr key;
    };

    Params parse_params_file(io::BaseByteStream &input_stream);

    void decrypt(
        io::BaseByteStream &input_stream, const common::Params &params);

} } } }
