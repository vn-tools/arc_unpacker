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
            bool is_recognized_internal(File &) const override;
            std::unique_ptr<File> decode_internal(File &) const override;
        };
    }
}

#endif
