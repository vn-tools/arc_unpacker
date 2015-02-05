#ifndef FORMATS_GFX_PRS_CONVERTER_H
#define FORMATS_GFX_PRS_CONVERTER_H
#include "formats/converter.h"

class PrsConverter final : public Converter
{
public:
    bool decode_internal(VirtualFile *) override;
};

#endif
