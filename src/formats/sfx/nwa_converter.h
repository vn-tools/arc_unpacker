#ifndef FORMATS_GFX_NWA_CONVERTER_H
#define FORMATS_GFX_NWA_CONVERTER_H
#include "formats/converter.h"

class NwaConverter final : public Converter
{
public:
    void decode_internal(VirtualFile &target_file) const override;
};

#endif
