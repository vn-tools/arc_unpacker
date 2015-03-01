#ifndef FORMATS_KEY_G00_CONVERTER_H
#define FORMATS_KEY_G00_CONVERTER_H
#include "formats/converter.h"

namespace Formats
{
    namespace Key
    {
        class G00Converter final : public Converter
        {
        public:
            void decode_internal(File &target_file) const override;
        };
    }
}

#endif
