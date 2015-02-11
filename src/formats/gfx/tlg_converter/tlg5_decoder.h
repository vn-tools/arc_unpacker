#ifndef FORMATS_GFX_TLG_CONVERTER_TLG5_DECODER_H
#define FORMATS_GFX_TLG_CONVERTER_TLG5_DECODER_H
#include "virtual_file.h"

class Tlg5Decoder final
{
public:
    void decode(VirtualFile &file);
};

#endif
