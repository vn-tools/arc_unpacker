#ifndef FORMATS_GFX_SPB_CONVERTER_H
#define FORMATS_GFX_SPB_CONVERTER_H
#include "formats/converter.h"

class SpbConverter final : public Converter
{
public:
    void decode_internal(VirtualFile &target_file) const override;
};

#endif
