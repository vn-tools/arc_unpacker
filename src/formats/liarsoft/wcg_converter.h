#ifndef FORMATS_LIARSOFT_WCG_CONVERTER_H
#define FORMATS_LIARSOFT_WCG_CONVERTER_H
#include "formats/converter.h"

namespace Formats
{
    namespace LiarSoft
    {
        class WcgConverter final : public Converter
        {
        protected:
            void decode_internal(File &target_file) const override;
        };
    }
}

#endif
