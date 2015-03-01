#ifndef FORMATS_LIZSOFT_SOTES_CONVERTER_H
#define FORMATS_LIZSOFT_SOTES_CONVERTER_H
#include "formats/converter.h"

namespace Formats
{
    namespace Lizsoft
    {
        class SotesConverter final : public Converter
        {
        public:
            void decode_internal(File &target_file) const override;
        };
    }
}

#endif
