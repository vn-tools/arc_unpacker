#ifndef FORMATS_RPGMAKER_XYZ_CONVERTER_H
#define FORMATS_RPGMAKER_XYZ_CONVERTER_H
#include "formats/converter.h"

namespace Formats
{
    namespace RpgMaker
    {
        class XyzConverter final : public Converter
        {
        protected:
            bool is_recognized_internal(File &) const override;
            std::unique_ptr<File> decode_internal(File &) const override;
        };
    }
}

#endif
