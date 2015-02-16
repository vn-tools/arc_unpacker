#ifndef FORMATS_GFX_G00_CONVERTER_H
#define FORMATS_GFX_G00_CONVERTER_H
#include "formats/converter.h"

class G00Converter final : public Converter
{
public:
    void decode_internal(File &target_file) const override;
};

#endif
