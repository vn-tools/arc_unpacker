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
            std::unique_ptr<File> decode_internal(File &) const override;
        };
    }
}

#endif
