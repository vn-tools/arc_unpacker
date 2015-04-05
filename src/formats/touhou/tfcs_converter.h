#ifndef FORMATS_TOUHOU_TFCS_CONVERTER_H
#define FORMATS_TOUHOU_TFCS_CONVERTER_H
#include "formats/converter.h"

namespace Formats
{
    namespace Touhou
    {
        class TfcsConverter : public Converter
        {
        protected:
            std::unique_ptr<File> decode_internal(File &) const override;
        };
    }
}

#endif
