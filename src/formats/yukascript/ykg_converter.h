#ifndef FORMATS_YUKASCRIPT_YKG_CONVERTER_H
#define FORMATS_YUKASCRIPT_YKG_CONVERTER_H
#include "formats/converter.h"

namespace Formats
{
    namespace YukaScript
    {
        class YkgConverter final : public Converter
        {
        protected:
            std::unique_ptr<File> decode_internal(File &) const override;
        };
    }
}

#endif
