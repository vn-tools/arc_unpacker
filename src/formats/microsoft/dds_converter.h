#ifndef FORMATS_MICROSOFT_DDS_CONVERTER_H
#define FORMATS_MICROSOFT_DDS_CONVERTER_H
#include "formats/converter.h"

namespace Formats
{
    namespace Microsoft
    {
        class DdsConverter final : public Converter
        {
        protected:
            bool is_recognized_internal(File &) const override;
            std::unique_ptr<File> decode_internal(File &) const override;
        };
    }
}

#endif
