#ifndef FORMATS_NSCRIPTER_SPB_CONVERTER_H
#define FORMATS_NSCRIPTER_SPB_CONVERTER_H
#include "formats/converter.h"

namespace Formats
{
    namespace NScripter
    {
        class SpbConverter final : public Converter
        {
        protected:
            void decode_internal(File &target_file) const override;
        };
    }
}

#endif
