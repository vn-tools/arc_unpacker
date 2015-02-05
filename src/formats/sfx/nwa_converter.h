#ifndef FORMATS_GFX_NWA_CONVERTER_H
#define FORMATS_GFX_NWA_CONVERTER_H
#include "formats/converter.h"

class NwaConverter final : public Converter
{
public:
    bool decode_internal(VirtualFile *) override;
};

#endif
