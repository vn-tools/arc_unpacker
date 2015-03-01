#ifndef FORMATS_RPGMAKER_XYZ_CONVERTER_H
#define FORMATS_RPGMAKER_XYZ_CONVERTER_H
#include "formats/converter.h"

namespace Formats
{
    namespace RpgMaker
    {
        class XyzConverter final : public Converter
        {
        public:
            void decode_internal(File &target_file) const override;
        };
    }
}

#endif
