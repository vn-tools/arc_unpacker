#ifndef AU_FMT_KID_DECOMPRESSOR_H
#define AU_FMT_KID_DECOMPRESSOR_H
#include "types.h"

namespace au {
namespace fmt {
namespace kid {

    bstr decompress(const bstr &input, size_t size_original);

} } }

#endif
