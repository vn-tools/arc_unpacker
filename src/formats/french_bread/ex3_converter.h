#ifndef FORMATS_FRENCH_BREAD_EX3_CONVERTER_H
#define FORMATS_FRENCH_BREAD_EX3_CONVERTER_H
#include "formats/converter.h"

namespace Formats
{
    namespace FrenchBread
    {
        class Ex3Converter final : public Converter
        {
        protected:
            std::unique_ptr<File> decode_internal(File &) const override;
        };
    }
}

#endif
