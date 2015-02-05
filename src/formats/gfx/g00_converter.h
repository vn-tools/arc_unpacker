#ifndef FORMATS_GFX_G00_CONVERTER_H
#define FORMATS_GFX_G00_CONVERTER_H
#include "formats/converter.h"

class G00Converter final : public Converter
{
public:
    bool decode_internal(VirtualFile *) override;
};

#endif
