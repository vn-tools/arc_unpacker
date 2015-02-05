#ifndef FORMATS_GFX_MGD_CONVERTER_H
#define FORMATS_GFX_MGD_CONVERTER_H
#include "formats/converter.h"

class MgdConverter final : public Converter
{
public:
    bool decode_internal(VirtualFile *) override;
};

#endif
