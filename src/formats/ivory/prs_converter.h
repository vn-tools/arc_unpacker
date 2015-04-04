#ifndef FORMATS_IVORY_PRS_CONVERTER_H
#define FORMATS_IVORY_PRS_CONVERTER_H
#include "formats/converter.h"

namespace Formats
{
    namespace Ivory
    {
        class PrsConverter final : public Converter
        {
        protected:
            void decode_internal(File &target_file) const override;
        };
    }
}

#endif
