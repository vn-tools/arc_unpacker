#ifndef FORMATS_GFX_BGI_CONVERTER_H
#define FORMATS_GFX_BGI_CONVERTER_H
#include "formats/converter.h"

class BgiConverter final : public Converter
{
public:
    void decode_internal(VirtualFile &target_file) const override;
};

#endif
