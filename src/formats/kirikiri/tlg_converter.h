#ifndef FORMATS_KIRIKIRI_TLG_CONVERTER_H
#define FORMATS_KIRIKIRI_TLG_CONVERTER_H
#include "formats/converter.h"

namespace Formats
{
    namespace Kirikiri
    {
        class TlgConverter final : public Converter
        {
        protected:
            void decode_internal(File &target_file) const override;
        };
    }
}

#endif
