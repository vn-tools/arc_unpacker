#ifndef AU_FMT_GLIB2_GML_DECODER_H
#define AU_FMT_GLIB2_GML_DECODER_H
#include "types.h"

namespace au {
namespace fmt {
namespace glib {

    class GmlDecoder
    {
    public:
        static bstr decode(const bstr &source, size_t target_size);
    };

} } }

#endif
