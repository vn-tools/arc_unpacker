#pragma once

#include "fmt/entis/common/decoder.h"
#include "io/bit_reader.h"

namespace au {
namespace fmt {
namespace entis {
namespace common {

    int get_gamma_code(io::BitReader &bit_reader);

} } } }
