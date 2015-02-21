#ifndef FORMATS_GFX_WCG_CONVERTER_H
#define FORMATS_GFX_WCG_CONVERTER_H
#include "formats/converter.h"

class WcgConverter final : public Converter
{
public:
    void decode_internal(File &target_file) const override;
};

#endif
