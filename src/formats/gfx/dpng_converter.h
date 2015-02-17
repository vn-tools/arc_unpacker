#ifndef FORMATS_GFX_DPNG_CONVERTER_H
#define FORMATS_GFX_DPNG_CONVERTER_H
#include "formats/converter.h"

class DpngConverter final : public Converter
{
public:
    void decode_internal(File &target_file) const override;
};

#endif
