#ifndef FORMATS_GFX_EX3_CONVERTER_H
#define FORMATS_GFX_EX3_CONVERTER_H
#include "formats/converter.h"

class Ex3Converter final : public Converter
{
public:
    void decode_internal(File &target_file) const override;
};

#endif
