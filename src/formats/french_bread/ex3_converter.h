#ifndef FORMATS_FRENCH_BREAD_EX3_CONVERTER_H
#define FORMATS_FRENCH_BREAD_EX3_CONVERTER_H
#include "formats/converter.h"

namespace Formats
{
    namespace FrenchBread
    {
        class Ex3Converter final : public Converter
        {
        public:
            void decode_internal(File &target_file) const override;
        };
    }
}

#endif
