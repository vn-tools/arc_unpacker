#ifndef FORMATS_TOUHOU_TFWA_CONVERTER_H
#define FORMATS_TOUHOU_TFWA_CONVERTER_H
#include "formats/converter.h"
#include "formats/touhou/palette.h"

namespace Formats
{
    namespace Touhou
    {
        class TfwaConverter : public Converter
        {
        protected:
            bool is_recognized_internal(File &) const override;
            std::unique_ptr<File> decode_internal(File &) const override;
        };
    }
}

#endif
