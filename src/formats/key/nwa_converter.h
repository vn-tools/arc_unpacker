#ifndef FORMATS_KEY_NWA_CONVERTER_H
#define FORMATS_KEY_NWA_CONVERTER_H
#include "formats/converter.h"

namespace Formats
{
    namespace Key
    {
        class NwaConverter final : public Converter
        {
        protected:
            void decode_internal(File &target_file) const override;
        };
    }
}

#endif
