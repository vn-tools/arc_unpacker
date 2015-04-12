#ifndef FORMATS_LIARSOFT_WCG_CONVERTER_H
#define FORMATS_LIARSOFT_WCG_CONVERTER_H
#include "formats/converter.h"

namespace Formats
{
    namespace LiarSoft
    {
        class WcgConverter final : public Converter
        {
        protected:
            bool is_recognized_internal(File &) const override;
            std::unique_ptr<File> decode_internal(File &) const override;
        };
    }
}

#endif
