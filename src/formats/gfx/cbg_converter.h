#ifndef FORMATS_GFX_CBG_CONVERTER_H
#define FORMATS_GFX_CBG_CONVERTER_H
#include "formats/converter.h"

class CbgConverter final : public Converter
{
public:
    void decode_internal(File &target_file) const override;
};

#endif
