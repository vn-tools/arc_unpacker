#ifndef FORMATS_BGI_CBG_CONVERTER_H
#define FORMATS_BGI_CBG_CONVERTER_H
#include "formats/converter.h"

namespace Formats
{
    namespace Bgi
    {
        class CbgConverter final : public Converter
        {
        protected:
            std::unique_ptr<File> decode_internal(File &) const override;
        };
    }
}

#endif
