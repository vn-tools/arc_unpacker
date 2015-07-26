#ifndef AU_FMT_GLIB2_GML_DECODER_H
#define AU_FMT_GLIB2_GML_DECODER_H
#include "io/buffered_io.h"

namespace au {
namespace fmt {
namespace glib {

    class GmlDecoder
    {
    public:
        static void decode(
            io::BufferedIO &source_io, io::BufferedIO &target_io);
    };

} } }

#endif
