#ifndef FORMATS_GFX_TLG_CONVERTER_H
#define FORMATS_GFX_TLG_CONVERTER_H
#include "formats/converter.h"

class TlgConverter final : public Converter
{
public:
    void decode_internal(VirtualFile &target_file) const override;
};

#endif

