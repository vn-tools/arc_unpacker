#ifndef FORMATS_QLIE_DPNG_CONVERTER_H
#define FORMATS_QLIE_DPNG_CONVERTER_H
#include "formats/converter.h"

namespace Formats
{
    namespace QLiE
    {
        class DpngConverter final : public Converter
        {
        public:
            void decode_internal(File &target_file) const override;
        };
    }
}

#endif
