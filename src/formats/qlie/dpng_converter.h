#ifndef FORMATS_QLIE_DPNG_CONVERTER_H
#define FORMATS_QLIE_DPNG_CONVERTER_H
#include "formats/converter.h"

namespace Formats
{
    namespace QLiE
    {
        class DpngConverter final : public Converter
        {
        protected:
            bool is_recognized_internal(File &) const override;
            std::unique_ptr<File> decode_internal(File &) const override;
        };
    }
}

#endif
