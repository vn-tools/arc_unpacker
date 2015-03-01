#ifndef FORMATS_YUKASCRIPT_YKG_CONVERTER_H
#define FORMATS_YUKASCRIPT_YKG_CONVERTER_H
#include "formats/converter.h"

namespace Formats
{
    namespace YukaScript
    {
        class YkgConverter final : public Converter
        {
        public:
            void decode_internal(File &target_file) const override;
        };
    }
}

#endif
