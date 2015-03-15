#ifndef FORMATS_GLIB2_GML_DECODER_H
#define FORMATS_GLIB2_GML_DECODER_H
#include "buffered_io.h"

class GmlDecoder
{
public:
    static void decode(BufferedIO &source_io, BufferedIO &target_io);
};

#endif
