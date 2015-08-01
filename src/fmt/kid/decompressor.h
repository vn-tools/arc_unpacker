#ifndef AU_FMT_KID_DECOMPRESSOR_H
#define AU_FMT_KID_DECOMPRESSOR_H
#include <memory>
#include "io/io.h"

namespace au {
namespace fmt {
namespace kid {

    std::unique_ptr<io::IO> decompress(io::IO &input_io, size_t size_original);

} } }

#endif
