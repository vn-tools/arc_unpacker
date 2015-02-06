#ifndef FORMATS_GFX_XYZ_CONVERTER_H
#define FORMATS_GFX_XYZ_CONVERTER_H
#include "formats/converter.h"

class XyzConverter final : public Converter
{
public:
    bool decode_internal(VirtualFile &) override;
};

#endif
