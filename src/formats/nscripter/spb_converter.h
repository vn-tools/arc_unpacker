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
            bool is_recognized_internal(File &) const override;
            std::unique_ptr<File> decode_internal(File &) const override;
        };
    }
}

#endif
